/* Simple Distance Sensor Program */

/* Basic principle of the HC-SR04 sensor is:
 *  From datasheet, basic working is:
 *  (A) Using IO trigger for at least 10us high level signal,
 *  (B) The Module automatically sends eight 40 kHz and detect whether there is a pulse signal back.
 *  (C) IF the signal back, through high level , time of high output IO duration is the time from sending ultrasonic to returning.
 *  (D)Test distance = (high level time×velocity of sound (340M/S) / 2,  Formula: uS / 58 = centimeters or uS / 148 =inch; 
 *  
 *  So, what we do is the following:
 *  1. We send a pulse of around 12 ms to the trigger pin of the sensor to start the transmission of the signal. 
 *  2. When the pulse is reflected back, we need to find hte time period of the reflected pulse. We do so, by setting an interrupt whenever there is a logic change. When there is a logic change from low to high, we start a timer and when there is a logic change from high to low, we stop the timer. This value is then stored.
 *  3. Using this value, we can calculate the distance of the object using the formula given. 
 */

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

volatile int t_count;
volatile int i = 0;

int main (void)
{
  //Initialisation of the required ports
  DDRD |= (1<<PIND7); //Trigger pin is output pin, and Echo pin is the source of external interrupt 
    
  //Initialisation of datacommunication
  Serial.begin(9600);

  //Initialisation of the interrupts
  EIMSK |= (1 << INT0);     //Same as GICR, sets up INT0 as the external interrupt
  EICRA |= (1 << ISC00);    //Setting up interrupt for any logic change
  sei();

  //Initialisation of the local variables
  uint32_t distance;
  
  while(1)
  {
    PORTD |= (1<<PIND7); //Setting the trigger pin high for 15 microseconds so that it'll send the signals
    _delay_us(15);
    PORTD &= ~(1<<PIND7); 
    distance = (t_count/58); //Formula for the distance in centimetres
    if (distance < 400)
    {
      Serial.print("Distance is = ");
      Serial.println(distance);
    }
  }
}

ISR(INT0_vect)
{
  if (i == 1) //Logic change from high to low
  {
    TCCR0B = 0; //Disable the clock
    if (TCNT0 > 3) //Checking for error since the minimum distance that can be measured by thte sensor is 3 cm
    {
      t_count = TCNT0; //Saving the count
      TCNT0 = 0; //Resetting TCNT0       
    }     
    t_count *= 64; //Multiplying by 64 to get the time period in microseconds
    i = 0;
  }
  
  if (i == 0) //Logic change from low to high
  {
    TCCR0B = (1<<CS02)|(1<<CS00); //Prescaling by 1024
    i = 1;
  }
  
} 

