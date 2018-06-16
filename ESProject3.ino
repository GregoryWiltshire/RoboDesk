/* defines pins numbers 
const int trigPin = 9; 
const int echoPin = 10;*/ 
const int K1 = 8;
const int K2 = 9;
const int K3 = 10; 
const int K4 = 11;
const int LEFT_LIMIT_SWITCH = 2;
const int RIGHT_LIMIT_SWITCH = 4;
const double CMIN16INCHES = 40.64;

#define trigger1 12 // Trigger Pin
#define echo1 13 // Echo Pin

int time;
int inRange = 45; // Wide Range First sight of Target
int TargetRange = 12; // Minimal Parking Range to Target
long duration, distance, lastDuration;
const int NoiseReject = 25; // Percentage of reading closeness for rejection filter
const unsigned int maxDuration = 11650; // around 200 cm, the sensor gets flaky at greater distances.
const long speed_of_sound = 29.1;    // speed of sound microseconds per centimeter



/* defines variables 
long duration; 
int distance; 
boolean on=false; */

// a function that returns the distance from the sonar sensor to nearest object in cm
long sonarDistance() {
  digitalWrite(trigger1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger1, LOW);
  duration = pulseIn(echo1, HIGH);
  // unfiltered = (duration / 2) / speed_of_sound; // Stores preliminary reading to compare
  if(duration <= 8) duration = ((inRange + 1) * speed_of_sound * 2); 
  // Rejects very low readings, kicks readout to outside detection range
  if(lastDuration == 0) lastDuration = duration;
  // Compensation parameters for intial start-up
  if(duration > (5 * maxDuration)) duration = lastDuration;
  // Rejects any reading defined to be out of sensor capacity (>1000)
  // Sets the fault reading to the last known "successful" reading
  if(duration > maxDuration) duration = maxDuration;  
  // Caps Reading output at defined maximum distance (~200)
  if((duration - lastDuration) < ((-1) * (NoiseReject / 100) * lastDuration)){
    distance = (lastDuration / 2) / speed_of_sound; // Noise filter for low range drops
  }
  distance = (duration / 2) / speed_of_sound;
  lastDuration = duration; // Stores "successful" reading for filter compensation
  return distance;
}


// if has reached height limit on limitswitch then will return true, else will return false
bool leftLimitReached(){
  if(digitalRead(LEFT_LIMIT_SWITCH)){
    return true;
  }
  else
    return false;
}

// if has reached height limit on limitswitch then will return true, else will return false
bool rightLimitReached(){
  if(digitalRead(RIGHT_LIMIT_SWITCH)){
    return true;
  }
  else
    return false;
}

// returns true if relay is engaged, false if not
bool isRelayOn(int pin){
  // need to return the logical negation
  return(
    !(digitalRead(pin))
   );
}

/* function to turn relay on, takes input relay pin#
required some safety code to ensure K1 and K2 will never be engaged simultaneously */
void turnRelayOn(int pin){
  // if relay1 is on and you're trying to turn on relay2 then return
  if(isRelayOn(K1) && pin == K2){
    Serial.println("you just tried to short circuit K1 and K2");
    return;
  }
  // if relay2 is on and you're trying to turn on relay1 then return
  if(isRelayOn(K2) && pin == K1){
    Serial.println("you just tried to short circuit K1 and K2");
    return;
  }
  
  // if relay1 is on and you're trying to turn on relay2 then return
  if(isRelayOn(K3) && pin == K4){
    Serial.println("you just tried to short circuit K3 and K4");
    return;
  }
  // if relay2 is on and you're trying to turn on relay1 then return
  if(isRelayOn(K4) && pin == K3){
    Serial.println("you just tried to short circuit K3 and K4");
    return;
  }

  // turn the relay on
  digitalWrite(pin, LOW);
}

// function to turn relay off, takes input relay pin#
void turnRelayOff(int pin){
  digitalWrite(pin, HIGH); 
}

// turns all relays off
void stop(){
  turnRelayOff(K1);
  turnRelayOff(K2);
  turnRelayOff(K3);
  turnRelayOff(K4);
  Serial.println("table is now stopped");
}

// need to delay k2 Right 
void tableUp(){
  // checks if the 2 down relays are off before turning on the 2 up relays with a delay before each one is turned on
  if(isRelayOn(K1)==0 && isRelayOn(K3)==0){
    // delay 15ms
    delay(15);    
    turnRelayOn(K4);
    delay(10);
    turnRelayOn(K2);
  }
  Serial.println("table is now going up");
}


void tableDown(){ // checks if the 2 up relays are off & then turns on the 2 down relays with a delay in-between each relay turning on
  if(isRelayOn(K2)==0 && isRelayOn(K4)==0){
    turnRelayOn(K1);
    delay(20);
    turnRelayOn(K3);
  }
}

void timedTableUp(int time){ // this function allows the table to move up after a specified amount of time 
  time = time - 15;
  tableUp();
  delay(time);
  stop();
}

void reset(){
  // gets the table down until one switch hits
  tableDown();
  Serial.println(rightLimitReached());
  while(!(rightLimitReached()) && !(leftLimitReached()));
  stop();
  
  if(!rightLimitReached()){
    turnRelayOn(K1);
    // busywait
    while(!rightLimitReached());
    stop();
  }

  if(!leftLimitReached()){
    turnRelayOn(K3);
    // busywait
    while(!leftLimitReached());
    stop();
  }
}

bool movement(){ // this section of the code implements the ultrasonic sensor in a way that allows it to detect a hand waved in front of it
 
  long temp1, temp2,temp3,temp4;
  temp1 = sonarDistance();
  delay(48);
  temp2 = sonarDistance();
    
  if((temp1+temp2)<=50){
    delay(70);
    temp3 = sonarDistance();
    delay(20);
    temp4 = sonarDistance();
      if((temp1+temp2)<=50){
       
        Serial.println("WAVE");
        return true;
      }
   }
   else{
    return false;
   }
}
 
void setup() {
  /* pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output 
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input */
  pinMode(K1, OUTPUT); 
  pinMode(K2, OUTPUT); 
  pinMode(K3, OUTPUT); 
  pinMode(K4, OUTPUT);
  pinMode(LEFT_LIMIT_SWITCH, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT_SWITCH, INPUT_PULLUP);
  pinMode(trigger1, OUTPUT);
  pinMode(echo1, INPUT);

  // Start with all relays OFF
  turnRelayOff(K1);
  turnRelayOff(K2);
  turnRelayOff(K3);
  turnRelayOff(K4);

  // start serial
  Serial.begin(9600);
  Serial.println("--- Start Serial Monitor ---");
}

void loop() {
  /* if there's been movement
  if(movement()){
  Start moving the desk
    
  }

  read in command from serial monitor */
  int Dword;
  Dword=Serial.read();
  // start recording how long the time was
  int temptime = millis();

// detect the gesture for a double hand wave
  if(movement()){
    // save time till second wave
    time = millis()-temptime;
    /*Serial.println(time);
    small delay
    Serial.println(time); */
     if(movement()){
      
        if(movement() && time<=200 && time>=100){
         // Serial.println(rightLimitReached()||leftLimitReached());
          if(rightLimitReached()||leftLimitReached()){
            timedTableUp(32400);
          }
          else {
            reset();
          }
          delay(2000);
        } 
     }
  }

  /* switch is closed, 1==closed 0==open
  if(digitalRead(RIGHT_LIMIT_SWITCH)==1){
    Serial.println("right closed");
  }
  
  switch is closed, 1==closed 0==open
  if(digitalRead(LEFT_LIMIT_SWITCH)==1){
    Serial.println("left closed");
  }*/

  if(Dword=='q'){ // the code beginning from this line checks the user's input & calls the corresponding function to move the table accordingly: up, down, reset, timed
    Serial.println("upleft");
    turnRelayOn(K4);
    delay(1000);
    stop();
  }
  if(Dword=='a'){
    Serial.println("downleft");
    turnRelayOn(K3);
    delay(1000);
    stop();
  }
  
  if(Dword=='e'){
    Serial.println("upright");
    turnRelayOn(K2);
    delay(1000);
    stop();
   
  }
  if(Dword=='d'){
    Serial.println("downright");
    turnRelayOn(K1);
    delay(1000);
    stop();
  }
  
  if(Dword=='o'){
    timedTableUp(32500);
  }
  
  if(Dword=='r'){
    reset();
  }
  
  if(Dword=='v'){
    timedTableUp(500);
  }
}
