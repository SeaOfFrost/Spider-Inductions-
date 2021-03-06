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

void LED_LightUp (int);
void LED_Wicket (void);


int main (void)
{
  //Initialization of variables
  int t_count = 0;
  int b_count[2];
  int count[2];
  int i = 0; 
  int j = 0;
  int k = 0;
  int l = 0;
  int m = 0;
  int runs = 0;
  int Pressed_Confidence_Level[2]; //For button debouncing
  int Released_Confidence_Level[2];
  int Pressed[2];

  //Initialization of ports for buttons and LEDs
  DDRB = 0b00001111; //First four pins of port B are output
  PORTB = 0b00000000; //All the pins are set low
  DDRD = 0x00; //Setting all the pins as input pins (I'll only be using the last two pins though)
  PORTD = 0b11100000; //Setting the last three pins high {for the buttons)
  
  //Initialisation of timer
  TCCR1B = (1<<CS12) | (1<<CS10); //Using Prescalar clk/1024
  TCNT1 = 0;

  //Initialisation of USART
  USART_Init();

  while(1)
    {
      if (bit_is_clear(PIND, 7))  //Button 1 is pressed
      {
            //start the timer to 5 seconds
            PORTB = 0b0000000;
            TCNT1 = 0; //TCINT will start counting again from the beginning     
              
            while (t_count < 5)   //The timer hasn't reached 5 seconds
              {
                if (TCNT1 == 15624)
                  {
                    t_count++;
                    TCNT1 = 0;
                  }

                PORTB = 0b00001000;

                count[0] = 1;
                count[1] = 1;

                if (bit_is_clear(PIND, 6)) //Check if button is pressed
                    {
                      Pressed_Confidence_Level[0]++; //Increase Pressed Conficence
                      Released_Confidence_Level[0] = 0; //Reset released button confidence since there is a button press
                      count[0] = 1;
                      
                      if (Pressed_Confidence_Level[0] > 5000)
                        {
                          //Make sure that the button was released
                          if(Pressed[0] == 0)
                          {
                            b_count[0]++;                          //Increase count (upto 6)
                            Pressed[0] = 1;
                          }
                         Pressed_Confidence_Level[0] = 0; 
                        }
                    }
                    
                 else if (count[0] == 1) 
                  {
                    Released_Confidence_Level[0]++; //Increase released confidence
                    Pressed_Confidence_Level[0] = 0; //Reset pressed confidence level
              
                    if (Released_Confidence_Level[0]> 5000)
                      {
                        //Occurs when the button is not pressed
                        Pressed[0] = 0;
                        Released_Confidence_Level[0] = 0;
                        count[0] = 0;
                      }
                  }

                  
                if (bit_is_clear(PIND, 5)) //Check if button is pressed
                    {
                      Pressed_Confidence_Level[1]++; //Increase Pressed Conficence
                      Released_Confidence_Level[1] = 0; //Reset released button confidence since there is a button press
                      count[1] = 1;
                      
                      if (Pressed_Confidence_Level[1] > 5000)
                        {
                          //Make sure that the button was released
                          if(Pressed[1] == 0)
                          {
                            b_count[1]++;                          //Increase count (upto 6)
                            Pressed[1] = 1;
                          }
                         Pressed_Confidence_Level[1] = 0; 
                        }
                    }
                    
                 else if (count[1] == 1) 
                  {
                    Released_Confidence_Level[1]++; //Increase released confidence
                    Pressed_Confidence_Level[1] = 0; //Reset pressed confidence level
              
                    if (Released_Confidence_Level[1]> 5000)
                      {
                        //Occurs when the button is not pressed
                        Pressed[1] = 0;
                        Released_Confidence_Level[1] = 0;
                        count[1] = 0;
                      }
                  } 
                }
              
            PORTB &= 0b00000000;

            i = b_count[0];
            j = b_count[1];
            
            if (i != j)
                 {
                    
                    runs = runs + i;
                    
                    if (runs > 9)
                    Display_Runs(runs);

                    else 
                    {
                      unsigned char ini_runs;
                      ini_runs = runs + '60'; //Accounting for the ASCII values
                      USART_Transmit(ini_runs);
                      USART_Transmit('\n');
                      LED_LightUp(b_count[0]);
                    }
                    
                 }

           if (i == j)
                 {
                    runs = 0;
                    USART_Transmit('W');
                    USART_Transmit('\n');
                    LED_Wicket();
                 }
                 
             b_count[0] = 0; //Reset the count
             b_count[1] = 0;        
             t_count = 0;
      } 
    }
}

void LED_LightUp (int count)
{
  switch(count)
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
}

void LED_Wicket(void)
{
  PORTB = 0b00000000;
  
  for (char k=0; k<20; k++)
  {
    PORTB ^= 0b00000111;
    _delay_ms(100);
  }
}

void Display_Runs(int number)
{
  int digits;
  int c = 0; /* digit position */
  int n = number;
  int i;
  char j;
  
  while (n != 0)
  {
      n /= 10;
      c++;
  }
  
  int numberArray[c];
  digits = c;
  c = 0;    
  n = number;
  
  /* extract each digit */
  while (n != 0)
  {
      numberArray[c] = n % 10;
      n /= 10;
      c++;
  }

  for (i=digits-1; i>-1; i--)
  {
    j = numberArray[i] + '60';  //Accounting for the ASCII values
    USART_Transmit(j);
  }
  
  USART_Transmit('\n');
  LED_LightUp(number);
}

