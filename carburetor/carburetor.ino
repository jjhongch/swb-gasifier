#include <Servo.h>
//Interrupt setup for measuring RPMs thank to Dale Grover
/////////////////////////////////////////////////
// NOTE!  millis() rolls over every 50 days!

// encoder phase A (phase B ignored) on pin D2
// want to have an external pull-up resistor, something like
// a 1k to 4.7K.  2.2K would be great.
#define encoderAPin 2

// count for encoder interrupt, starts at 0
unsigned long encoderCount;
// starting time for encoder count
unsigned long encoderStartTime;



// interrupt routine
// called when the encoder A signal goes high
void irqEncoder()
{
  encoderCount++; // just increment the count
} // irqEncoder()


// calculate RPM from encoder count
unsigned int getRPM()
{
  unsigned long count;
  unsigned long elapsedTime;
  unsigned long rpm;
  unsigned long timeNow;
  noInterrupts(); // turn off interrupts while read the value so it is not changed by an interrupt during read
  count=encoderCount;
  encoderCount=0L; // reset to 0
  interrupts(); // turn interrupts back on
  timeNow=millis();
  // how long since last read encoder count?
  elapsedTime=timeNow-encoderStartTime;
  rpm = (count * 100L) / elapsedTime;
  Serial.print("time=");
  Serial.println(elapsedTime);
  Serial.print("count=");
  Serial.println(count);
  encoderStartTime=timeNow;
  return(rpm); 
} // getRPM()

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//Setup Choke choke manual position
int use_Pot_Choke()
{
  //Variables for Potentiometer choke knob
 int potPin = A1;
 int potVal; //Variable that will read the potentiometer
 potVal = analogRead(potPin);
 potVal = map(potVal, 0, 1023, 120, 180);
 return(potVal);
}
/////////////////////////////////////////////////////////////////////////////

// Variables for Throttle and Choke Servos
const int throttle = 5;
const int choke = 7;
Servo throttleServo; //Fully closed is 180 degrees and fully open is 120 degrees
Servo chokeServo;//Fully closed is 120 and fully open is 180, opposite of the throttle

int potPin = A1;

// Variables for Oxygen Sensor
const int O2_Sensor = 0;
int digital_volts;
float volts;
float air_fuel_ratio;

int chokePosition = 120; //initiates the position of the servo at fully closed and 120
int throttlePosition = 120; //initiates the position of the Throttle at fully open or 120;



void setup() {
  Serial.begin(9600);
  
  //Set up the RPM encoder
pinMode(encoderAPin,INPUT_PULLUP); // we still want a 2.2K
  // initialize the encoder start time & count
  encoderStartTime=0L;
  encoderCount=0L;
  // enable interrupts
  attachInterrupt(digitalPinToInterrupt(encoderAPin), irqEncoder, RISING);
  

  // Setup Servos 
  throttleServo.attach(throttle);
  throttleServo.write(throttlePosition);
  chokeServo.attach(choke);
  chokeServo.write(180);

 

}

void loop() {

//Monitor the RPMs  
Serial.print("RPM is ");
  Serial.println(getRPM());
  

//Read O2 Sensor
  digital_volts = analogRead(O2_Sensor);
  volts = 5.0 * (digital_volts/1024);
  air_fuel_ratio = (2.375*volts) + 7.3125; // Just for interest - book recommends controlling on voltage
//End read Sensor

//Control loop for engine speed
//Throttle is fully closed at 180 and fully open at 120
int currentRPM = getRPM();
 if (currentRPM > 3650 && throttlePosition < 180) {
  throttlePosition++;
  throttleServo.write(throttlePosition);
 }
 else if ( currentRPM < 3550 && throttlePosition > 120) {
  throttlePosition--;
  throttleServo.write(throttlePosition);
 }
 //End of control of engine speed



 //Control Loop for Choke
 if (analogRead(potPin) < 1000) {
  chokeServo.write(use_Pot_Choke());
 }
 else {
  if (volts < 2.33 && chokePosition > 120) {
    chokePosition--;//Fully closed is 120 and fully open is 180, opposite of the throttle
    chokeServo.write(chokePosition);
  }
  if (volts > 3.13 && chokePosition < 180) {
    chokePosition++;
    chokeServo.write(chokePosition);
  }
 }

 
}
