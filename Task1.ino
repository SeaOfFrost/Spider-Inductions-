#include<avr/io.h>
#include<util/delay.h>

void USART_Init()
{
 //Set baud rate 
 UBRR0L = 0xCF; //Set the baud rate as 9600
 //Enable receiver and transmitter 
 UCSR0B = (1<<RXEN0)|(1<<TXEN0);
 // Set frame format: 8 data, 2 stop bit 
 UCSR0C = (1<<UCSZ00)|(1<<UCSZ01)|(1<<USBS0);
}

void USART_Transmit(char data)
{
 // Wait for empty transmit buffer 
 while (!(UCSR0A & (1<<UDRE0)));
 // Put data into buffer, sends the data 
 UDR0 = data;
}

unsigned char USART_Receive(void)
{
 // Wait for data to be received 
 while (!(UCSR0A & (1<<RXC0)));
 // Get and return received data from buffer 
 return UDR0;
}

int main (void)
{
  //Initialization of variables
  int t_count = 0;
  char b_count = '0';
  int Pressed = 0;
  int Pressed_Confidence_Level[2]; //For button debouncing
  int Released_Confidence_Level[2];

  //Initialization of ports for buttons and LEDs
  DDRB = 0b00011111; //First three pins of port B are output
  PORTB = 0b00000000; //All the pins are set low
  DDRD = 0x00; //Setting all the pins as input pins (I'll only be using the last two pins though)
  PORTD = 0b11000000; //Setting the last two pins high for (the buttons)
  
  //Initialisation of timer
  TCCR1B = (1<<CS12) | (1<<CS10); //Using Prescalar clk/1024
  TCNT1 = 0;

  //Initialisation of USART
  USART_Init();

  while(1)
    {
      if (bit_is_clear(PIND, 7))  //Button 1 is pressed
      {
        Pressed_Confidence_Level[0]++;
        if (Pressed_Confidence_Level[0] > 1000)
          {
            //start the timer to 5 seconds
            PORTB &= 0b00000000;
            TCNT1 = 0; //TCINT will start counting again from the beginning     
              
            while (t_count < 5)   //The timer hasn't reached 5 seconds
              {
                if (TCNT1 == 15624)
                  {
                    t_count++;
                    TCNT1 = 0;
                    PORTB = 0b00001000;
                  }

                if (bit_is_clear(PIND, 6)) //Check if button is pressed
                    {
                      Pressed_Confidence_Level[1]++; //Increase Pressed Conficence
                      Released_Confidence_Level[1] = 0; //Reset released button confidence since there is a button press
                      
                      if (Pressed_Confidence_Level[1] > 5000)
                        {
                          //Make sure that the button was released
                          if(Pressed == 0)
                          {
                            b_count++;                          //Increase count (upto 6)
                            PORTB ^= 0b00010000;
                            Pressed = 1;
                          }
                         Pressed_Confidence_Level[1] = 0; 
                        }
                    }
                    
                 else
                  {
                    Released_Confidence_Level[1]++; //Increase released confidence
                    Pressed_Confidence_Level[1] = 0; //Reset pressed confidence level
              
                    if (Released_Confidence_Level[1]> 5000)
                      {
                        //Occurs when the button is not pressed
                        Pressed = 0;
                        Released_Confidence_Level[1] = 0;
                      }
                  }
              }
             PORTB = 0b00000000;
             USART_Transmit(b_count);
             USART_Transmit('\n'); 
  
             switch(b_count)
                  {
                    case 1:                     
                    PORTB = 0b00000001;
                    break;
      
                    case 2:
                    PORTB = 0b00000010;
                    break;
      
                    case 3:
                    PORTB = 0b00000011;
                    break;
      
                    case 4:
                    PORTB = 0b00000100; 
                    break;
      
                    case 5:
                    PORTB = 0b00000101;
                    break;
      
                    case 6:
                    PORTB = 0b00000110;
                    break;
      
                    default:
                    PORTB = 0b00000000;
                    break;
                  }
                           
             b_count = '0'; //Reset the count        
             t_count = 0;
          }
      } 
    }
}
