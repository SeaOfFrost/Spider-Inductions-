#include<avr/io.h>
#include<util/delay.h>

/* *************************************************************************************************************************** */
/********************************************FUNCTION DEFINITIONS***************************************************************/

/* 
 * Note: Here, I'm using Pins B0, and Pin B1 as the two lines. 
 * Pin B0 is the SDA line
 * Pin B1 is the SCL line
 * 
 * The protocol is basically, that the SDA line is always high, unless there is a data transfer. (ie. when the system is idle, the SDA line is high)
 * In this protocol, similar to I2C, the dataline doesn't change when the clock line is high, however, it changes only for the start and the stop functions.  
 */

void timer1_init()
{
  TCCR1B = (1<<CS12); //Prescaling by 256
  TCNT1 = 0;
}

/*START Function is defined by the SCL going low for 20 ms, while the SDA line has to go low */
void proto_Start(void) 
{
  //Start TCNT1
  TCNT1 = 0; 
  while (TCNT1 < 1250) //Keeping the signal the same for 20ms, so that the receiver MCU's will receive the START signal 
  PORTB &= ~(0x03); //Setting the clock line and the data line low (data line is to be kept low for data transfer)
}

/*STOP Function is defined by the SCL going high for 40 ms, while the SDA line is low*/
void proto_Stop(void)
{
  PORTB &= ~(0x01); //Setting the data line low, so as to enable data transfer
  PORTB |= 0x02; //Setting the clock line high
  TCNT1 = 0;
  while (TCNT1 < 2500) //Keeping the signal the same for 40ms, so that the receiver MCU's will receive the STOP signal    
  PORTB |= 0x01; //Returning the dataline to high 
}


/* TRANSMIT Function is defined by doing the following
 *  1. While keeping the SDA line low, the SCL line will be used to transmit the data   
 *  2. From the given 8 bit data (from the master), we find the values of each bit
 *  3. The values of each bit is then saved in an array
 *  4. Using the array, we send the required bit, by setting the SCL pin high (for 1) or low (for 0).
 */
 
void proto_Transmit(unsigned char data)
{
  PORTB &= ~(0x01); //Setting the SDA line low, so that the system is not idle 
  unsigned char i;
  
  /*Generally, to find the k-th bit of n, we use (n >> k) & 1
   * By right-shifting the desired bit into the least significant bit position, masking can be done with 1, to find the value. 
   */  
  //I use this loop to send the data bits as per our requirement through the clock line. The clock line will be set high if the bit is high and vice versa.
  for (i=0; i<8; i++, data >>= 1)
  {
    
    if (data & 1)
      PORTB |= 0x02;  //Setting the clock line high
    else       
      PORTB &= ~(0x02); //Setting the clock line low
    TCNT1 = 0;
    while (TCNT1 < 1250); //Keep the signal high/low for 20 ms
  }
  
  PORTB |= 0x01;
}

/* Function to check if an ACK message was sent by the slave
 * I do this by keeping a small check if the lines were pulled low by the slave
 */
unsigned char ACK_Check (void)
{
  unsigned char check;
  check = PINB & 0x03; //Checking for the ACK
  if (check != 0x03)
  return 1;
  else 
  return 0;
}

/* Function to simplify data transfer. Made to send on character
 * I do the same by:
 * (1) First, send the start condition
 * (2) Then, transmit the address of the slave
 * (3) Check if an ACK is sent by any of the slaves. If there was no ACK was sent, we resend the message
 * (4) If an ACK was sent, we then send the message character
 * (5) Once that is sent, then send the stop condition
 */
void proto_send_char (unsigned char Address, unsigned char Message)
{
  proto_Start(); 
  proto_Transmit(Address); //Send the address
  
  if (ACK_Check)
  {
    TCNT1 = 0;
    while (TCNT1 < 1250); //Wait for 20 ms
    proto_Transmit(Message);
    
    //Resend the character if the Ack was not sent
    while (!(ACK_Check))        
    proto_Transmit(Message);        
  }
  
  proto_Stop();
  _delay_ms(10);
}

/* *************************************************************************************************************************** */
int main (void)
{
  //Initialising the ports and keeping and both SCL and SDA pins high
  DDRB = 0xFF; //We set everything in Port D as output pins
  PORTB |= 0x03; //If they are outputs, we want them to be low. Except Pin B0, as it is the dataline pin which must be high for idle state. So, I also put SCL line high also.

  //Initialising the timer
  timer1_init();
  
  while(1)
  {
    proto_send_char(0x01, 0x26); //Sending character '&' to MCU who's address is 0x01
    proto_send_char(0x02, 0x53); //Sending character 'S' to MCU who's address is 0x02
  }
}

/* *************************************************************************************************************************** */
