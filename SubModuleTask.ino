/* Simple Tracking System Program */

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
 *  
 *  Basic Principle of how the motor works
 *  From the datasheet, basic working is:
 *  (A) To get rotation to 0 degree position, we give a high pulse for 1000 us 
 *  (B) To get rotation to 90 degree position, we give a high pulse for 1500 us 
 *  (C) To get rotation to 180 degree position, we give a high pulse for 2000 us 
 *  
 *  So, what we do here is:
 *  1. We set up the timer 1 to use PWM of mode 14. In this mode, the high pulse is generated till we reach the ICR1 value (which is the top value)
 *  2. If we fix the PWM frequency as 50kHz, and use prescalar of 64, and apply the formula, we get the top value (ie. ICR1 value) as 4999
 *  3. The formula is F(PWM) = F(CPU)/N(1+TOP) N is the prescalar
 *  4. When the prescalar, the clock ticks every 4 us. Using this, we can find the OCR1A value by dividing the required time by 4. EG: For 0 degree: 1000/4 hence, the OCR1A value is 250
*/

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

volatile int t_count;
volatile int i = 0;
volatile int angle;

int main (void)
{
  //Initialisation of the required ports
  DDRD |= (1<<PIND7); //Trigger pin is output pin, and Echo pin is the source of external interrupt 
  DDRB |= (1<<PINB1);   //PWM pin is output
  DDRC |= (1<<PINC0); //Buzzer is connected to Pin C0
    
  //Initialisation of datacommunication
  Serial.begin(9600);

  //Initialisation of the interrupts
  EIMSK |= (1 << INT0);     //Same as GICR, sets up INT0 as the external interrupt
  EICRA |= (1 << ISC00);    //Setting up interrupt for any logic change
  sei(); //Initializing global interrupts

  //Initialisation of Timer1 for PWM (for the motor)
  TCCR1A |= (1<<COM1A1)| (1<<COM1B1)| (1<<WGM11);        //Non inverted PWM
  TCCR1B |= (1<<WGM13)| (1<<WGM12)| (1<<CS11)| (1<<CS10); //Fast PWM with prescalar of 64
  ICR1 = 4999;  //Frequency of PWM is 50 kHz

  //Initialisation of the local variables
  unsigned char j,k;
  
  while(1)
  {   
    for (OCR1A = 250; OCR1A < 501;)  //OCR is 250 for 0 degreem 375 for 90 degree and 500 for 180 degree (To move the motor in one direction)
    {
      _delay_ms(2000);
      angle = 375 - OCR1A; 
      OCR1A += 50;
      FindDistance();
    }

    for (OCR1A = 500; OCR1A > 249;) //To move the motor in the other direction
    {
      _delay_ms(2000);
      angle = 375 - OCR1A; 
      OCR1A -= 50;
      FindDistance();
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

void FindDistance (void)
{
  PORTD |= (1<<PIND7); //Setting the trigger pin high for 15 microseconds so that it'll send the signals
  _delay_us(15);
  PORTD &= ~(1<<PIND7);   

  unsigned int distance;

  distance = t_count/58; //Formula for the distance in centimetres

  if (distance < 400 && distance > 0)
  {
    if (distance < 100)
    {
      Serial.print("Enemy is ");
      Serial.print(distance);
      Serial.println(" cm away"); 

      angle *= 18;
      angle /= 25;
      
      if (angle > 0)
      {
        Serial.print("Enemy is at around ");
        Serial.print(angle);
        Serial.println(" degrees to the right"); 
      }
      if (angle < 0)
      {
        angle *= -1;
        Serial.print("Enemy is at around ");
        Serial.print(angle);
        Serial.println(" degrees to the left"); 
      }
      FireBuzzer(); 
    } 
    else 
    {
      Serial.print("Nearest object is ");
      Serial.print(distance);
      Serial.println(" cm away"); 
    }
    distance = 0;
  }
}

void FireBuzzer (void) //Function to get the buzzer to beep when the obstacle is less than one metre away
{
  unsigned char count;
  
  for (count = 0; count < 5; count++) //We beep the buzzer 5 times
  {
    PORTC |= (1<<PINC0);
    _delay_ms(100);
    PORTC &= ~(1<<PINC0);
    _delay_ms(100);
  }
}


