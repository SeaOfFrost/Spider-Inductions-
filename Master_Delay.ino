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


void dly(int t_count)
{
  //Write a funtion to set a timer for the required milliseconds
}

/*START Function is defined by the SCL going low for 20 ms, while the SDA line has to go low */
void proto_Start(void) 
{
  PORTB &= ~(0x03); //Setting the clock line and the data line low (data line is to be kept low for data transfer)
  _delay_ms(20); //Keeping the signal the same for 20ms, so that the receiver MCU's will receive the START signal
}

/*STOP Function is defined by the SCL going high for 40 ms, while the SDA line is low*/
void proto_Stop(void)
{
  PORTB &= ~(0x01); //Setting the data line low, so as to enable data transfer
  PORTB |= 0x02; //Setting the clock line high
  _delay_ms(40); //Keeping the signal the same for 40ms, so that the receiver MCU's will receive the STOP signal
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
    _delay_ms(20);
  }
  
  PORTB |= 0x01;
}

/* *************************************************************************************************************************** */
int main (void)
{
  DDRB = 0xFF; //We set everything in Port D as output pins
  PORTB |= 0x03; //If they are outputs, we want them to be low. Except Pin B0, as it is the dataline pin which must be high for idle state.
  Serial.begin(9600);

  unsigned char a;
  
  while(1)
  {
    proto_Start(); 
    proto_Transmit(0x01); //Address of first MCU
    
    a = PINB & 0x02; //Checking for the ACK. We make an exception for the data transfer here. Only for ACK, can we transfer data while the SDA line is high
    if (a != 0x02)
    {
      _delay_ms(20);
      proto_Transmit(0x26);    //Transferring the character '&'
      
      a = PINB & 0x02;
      while (a == 0x02)        //Resend the character if the Ack was not sent
      {
        proto_Transmit(0x26);        
      }
    }

    else 
    break;

    proto_Stop();
    _delay_ms(10);

    proto_Start(); 
    proto_Transmit(0x02); //Address of Second MCU
    
    a = PINB & 0x02; //Checking for the ACK. We make an exception for the data transfer here. Only for ACK, can we transfer data while the SDA line is high
    if (a != 0x02)
    {
      _delay_ms(20);
      proto_Transmit(0x53);    //Transferring the character 'S'
      
      a = PINB & 0x02;
      while (a == 0x02)        //Resend the character if the Ack was not sent
      {
        proto_Transmit(0x53);        
      }
    }

    else 
    break;

    proto_Stop();
    _delay_ms(10);
  }
} 


