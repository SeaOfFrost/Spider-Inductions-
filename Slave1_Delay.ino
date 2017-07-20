#include<avr/io.h>
#include<util/delay.h>

/* *************************************************************************************************************************** */
/********************************************USART FUNCTION DEFINITIONS***************************************************************/
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void USART_Init()
{
 //Set baud rate 
 UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
 UBRR0L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
 //Enable receiver and transmitter 
 UCSR0B = (1<<RXEN0)|(1<<TXEN0);
 // Set frame format: 8 data, 2 stop bit 
 UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}

void USART_Transmit_Char(char data)
{
 // Wait for empty transmit buffer 
 while (!(UCSR0A & (1<<UDRE0)));
 // Put data into buffer, sends the data 
 UDR0 = data;
}

void USART_Transmit_String(char data[])
{
  unsigned char i;
  for (i = 0; data[i] != '\0'; i++)
  {
    // Wait for empty transmit buffer 
   while (!(UCSR0A & (1<<UDRE0)));
   // Put data into buffer, sends the data 
   UDR0 = data[i];
  }
}

/* *************************************************************************************************************************** */
/********************************************PROTOCOL FUNCTION DEFINITIONS***************************************************************/

/* 
 * Note: Here, I'm using Pins B0, and Pin B1 as the two lines. 
 * Pin B0 is the SDA line
 * Pin B1 is the SCL line
 * 
 * The protocol is basically, that the SDA line is always high, unless there is a data transfer. (ie. when the system is idle, the SDA line is high)
 * In this protocol, similar to I2C, the dataline doesn't change when the clock line is high, however, it changes only for the start and the stop functions.  
 */

unsigned char Receive(void)
{
  unsigned char a, i;
  unsigned char data = 0x00;

  while (bit_is_clear(PINB, 0) && i<8) //While the SDA line is low
  {    
    a = PINB & 0x02;

    if (a == 0x02)
      data |= 1<<i; //To set the bit high
    else 
      data &= ~(1<<i); //To set the bit low
      
    _delay_ms(20);
    i++; 
  }
  return data;
}

void Ack (void)
{
  PORTB &= ~(0x03);
  _delay_ms(20);
}

/************************************************************************************************/
int main (void)
{
  DDRD = 0xFF;
  int Confidence_Level;
  USART_Init();
  unsigned char a, b, c;
  int i = 0;

  while(1)
  {
    a = PINB & 0x03;

    if (a != 0x03)
    {
      _delay_ms(20);
      b = Receive();
      if (b == 0x01)  //The Address of the slave is 0x0C, however, the receive function only returns double of that value. No idea why the left shifting occurs.
      {
        Ack(); //Send the ACK
        b = Receive(); //Now, receive the transmitted data
        Ack();  
        i++;     
      }
    }

    //Write code for the stop function. Basically, show output only if the stop condition was sent. 
    //Code to see if the output is correct.
    c = PINB & 0x02;
    if (c == 0x02 && i == 1)
    {
      if (b != 0x00)
      USART_Transmit_Char(b);
      i = 0;       
    } 
  }
}

/******************************************************************************/


