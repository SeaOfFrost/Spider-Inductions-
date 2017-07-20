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
 // Set frame format: 8 data, 1 stop bit 
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
 *  
 * IMPORTANT NOTE: This is basically is made for the pins 0 (as the SDA) and 1 (as the SCL). To change accordingly, switch the port and pin name.
 */

/* Function for initialising the timer */
void Timer1_Init()
{
  TCCR1B = (1<<CS12); //Prescaling by 256
  TCNT1 = 0;
}

/* Function to receive data.
 * To receive data using three steps
 * (1) I do that by first keeping a check whether the SDA line is low
 * (2) The loop is used to set the required bits high/low based on whether the signal sent was high or low
 * (3) I then use a delay wait for the signal to change (20 ms)
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
    i++; 
    TCNT1 = 0;
    while (TCNT1 < 1250); //Wait for 20 ms
  }
  return data;
}

/* This is a function to receive data from the master. 
 * I do that through two steps: 
 * (1) First check both the SDA and SCL lines if they are high. We choose to receive data only when SDA line isn't high 
 * (2) Then receive data. If the received data is the same as the address, then we return 1
 */
unsigned char Address_Check (unsigned char Address) 
{
  unsigned char check, Received_Address;
  check = PINB & 0x03;

  if (check != 0x03) 
  {
    TCNT1 = 0;
    while (TCNT1 < 1250); //Wait for 20 ms
    //_delay_ms(20); //I'm using delay instead of a timer, because we reduce the accuracy by using a timer
    Received_Address = Receive();
    if (Received_Address == Address)
    return 1;
    else 
    return 0;
  }
}

/* This is the function to check for a stop condition
 * Basically, a small check whether only if the SCL line is low,
 */
unsigned char Stop_Check (void)
{
  unsigned char check;
  check = PINB & 0x02;
  if (check == 0x02)
  return 1;
  else
  return 0;
}

/* Function to simply send an ACK signal
 * We do that by keeping both SDA and SCL line low for 20 ms
 */
void Ack (void)
{
  //This is the only case where the slave will drive the system 
  PORTB &= ~(0x03); //Pulling the pins low
  TCNT1 = 0;
  while (TCNT1 < 1250); //Wait for 20 ms
}

/* This is a function is simply to receive a character from the master
 * I do this by the following steps:  
 * (1) First receive the address bits from the master, verify if the address of the slave is the same as the data sent by the master. If yes, we send an ACK.
 * (2) The slave then listens for another character from the master 
 * (3) We store the message and check if the stop condition was sent. 
 * (4) If the stop condition was sent, we use USART to show the output through the serial monitor
 */
void Receive_Char (unsigned char Address)
{
  unsigned char message;
  int i;
    
  if (Address_Check(Address))  //The Address of the slave is 0x02, we send an ack, only if the received address is the same
  {
    Ack(); //Send the ACK
    message = Receive(); //Now, receive the transmitted data
    Ack();  
    i++;     
  }

  //Code for check the stop function. Basically, show output only if the stop condition was sent. 
  if (Stop_Check && i == 1)
  {
    if (message != 0x00)
    USART_Transmit_Char(message);
    i = 0;       
  } 
} 

/* *************************************************************************************************************************** */
int main (void)
{
  //Initialising serial port datacommunication (USART)
  USART_Init();

  //Initialising timer 
  Timer1_Init();

  while(1)
  {
    Receive_Char(0x01); //The Address of the slave is 0x01
  }
}

/* *************************************************************************************************************************** */


