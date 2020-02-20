//global variables that are used throught multiple functions
//pins (counter and clockwise pins are correct)
int motorLocPin = 2; //buttonPIN
int motorLocState;//ButtonState
int counterPin = 4;
int clockwisePin = 6;
int motorEnablePin = 9; 
int limitSwitchPin = 8;
int armSolonoidPin = 11;
int suctionSolonoidPin = 12;
int photoTransistorPin = 7;
int compressorPin = 10;

//motor location inputs and outputs
volatile int motorLoc;
int clockwiseVal = 0;
int counterVal = 1;

//how many clicks has the motor turned since it has gone home
int loc = 0;

//(probably) not important, not being used right now
int turnMax = 5;

//how many clicks until we get to a certiain location
int blueLoc = 11;
int redLoc = 10;
int whiteLoc = 8;
int boxLoc = 7;

//how long to wait to see if a signal is a correct signal
int timeToWait = 8;
//debounce delay

//are we currently looking at the first interrupt
//boolean firstSignal = true;
long deblast=0; //lastdebouncetime interrupt

int whiteColorMin = 540;
int whiteColorMax = 570;

int blueColorMin = 680;
int blueColorMax = 710;

int redColorMin = 570;
int redColorMax = 590;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
///Interrupts
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

//interrupt function

void changeDirection() {
  //motorLocState=digitalRead(motorLocPin);
  if ((millis()-deblast)>timeToWait){
    loc++;
    deblast=millis();
  }
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
///Main Functions
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  //setting up pins
  pinMode(motorLocPin, INPUT);
  pinMode(counterPin, OUTPUT);
  pinMode(clockwisePin, OUTPUT);
  pinMode(motorEnablePin, OUTPUT);
  pinMode(limitSwitchPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(armSolonoidPin, OUTPUT);
  pinMode(photoTransistorPin, OUTPUT);
  pinMode(suctionSolonoidPin, INPUT);
  pinMode(compressorPin, INPUT);
  pinMode(A0, INPUT);

  //assigning interrupts
  attachInterrupt(digitalPinToInterrupt(motorLocPin), changeDirection, CHANGE);

  digitalWrite(motorEnablePin, HIGH);

  
  //find home
  findHome();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//main loop
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void loop() { 
  //fill with air
  digitalWrite(suctionSolonoidPin, HIGH);

  
  //debugging for color vals
  int colorMeasurments = analogRead(A0);
  //Serial.print("Color Val: ");
  //Serial.println(colorMeasurments);
  

  //see if we need to collect
  /*
  while ( digitalRead(photoTransistorPin) == LOW ) {
    //do nothing... there is nothing that needs to be done
    Serial.println("Puck is not present");
  }
  */


  
  //lower arm to collector and collect, rise.
  unsortedTray();

  //go to color sensor, lower, and find the measurment, rise
  int color = colorSensorfunction(colorMeasurments);

  //go to appropriate designattor, lower, release, rise
  sort(color);

  //go home
  findHome();

}



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
///Find Location
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void findHome () {
  //stop the motor
  motorStop();
  
  //turn until home found
  while ( digitalRead ( limitSwitchPin ) == LOW ) {
    motorCounter();
  }

  
  //stop!!
  motorStop();
  
  //set location to 0
  loc = 0;

  //go fowared
  motorClockwise();

  delay(5);

  //stop agian
  motorStop();
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
///MOTOr movements
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void motorStop () {
  counterVal= 0;
  clockwiseVal = 0;
  digitalWrite(counterPin, counterVal);
  digitalWrite(clockwisePin, clockwiseVal);
}

void motorClockwise (){
  counterVal= 0;
  clockwiseVal = 1;
  digitalWrite(counterPin, counterVal);
  digitalWrite(clockwisePin, clockwiseVal);
}


void motorCounter (){
  counterVal= 1;
  clockwiseVal = 0;
  digitalWrite(counterPin, counterVal);
  digitalWrite(clockwisePin, clockwiseVal);
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
///GOTO Location
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void findRed() {
  while ( loc <= redLoc ) {
    motorClockwise();
  }
  motorStop();
  Serial.println("Red Found");
  delay(1000);
}

void findBlue () {
  while ( loc <= blueLoc ) {
    motorClockwise();
  }
  Serial.println("Blue Found");
  motorStop();
  delay(1000);
}

void findWhite() {
  while ( loc <= whiteLoc ) {
    motorClockwise();
  }
  motorStop();
  Serial.println("White Found");
  delay(1000);
}

void findBox() {
  while ( loc <= boxLoc ) {
    motorClockwise();
  }
  motorStop();
  Serial.println("Box Found");
  delay(1000);
}

 
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//Color determination
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int determineColor(int colorVal) {
  int color = 0;
  if ( ( colorVal > whiteColorMin ) && ( colorVal < whiteColorMax ) ) {
    color = 1;
  }

  if ( ( colorVal > blueColorMin ) && ( colorVal < blueColorMax ) ) {
    color = 2;
  }

  if ( ( colorVal > redColorMin ) && ( colorVal < redColorMax ) ) {
    color = 3;
  }
  if ( color == 0 ) {
    Serial.println("Color not found");
  }

  return color;
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//activation of unsorted tray
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void unsortedTray() {
  motorStop();
  delay(1000);
  Serial.println("Stopped at pick-up");
  digitalWrite(suctionSolonoidPin, LOW);
  
  //lower arm
  digitalWrite(armSolonoidPin, HIGH);
  
  //suck bean
  delay(500);
  digitalWrite(suctionSolonoidPin, HIGH);
  delay(500);
  
  //raise arm
  digitalWrite(armSolonoidPin, LOW);
  delay(100);
}




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//color determinattion
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int colorSensorfunction(int colorMeasure) {
  //go to color sensor
  findBox();
  
  //lower arm
  digitalWrite(armSolonoidPin, HIGH);
  
  //determine color
  delay(250);
  int color = determineColor(colorMeasure);
  delay(250);

  //raise arm
  digitalWrite(armSolonoidPin, LOW);
  

  return color;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//sort
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void sort(int color) {
  if ( color == 1 ) {//white
    findWhite();

    
  }

  else if ( color == 2 ) {//blue
    findBlue();

    
  }

  else if ( color == 3 ) {//red
    findRed();

    
  }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//relase
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void releaseBean() {
  //lower arm
  digitalWrite(armSolonoidPin, HIGH);

  
  //release bean
  digitalWrite(suctionSolonoidPin, LOW);

  //raise arm
  digitalWrite(armSolonoidPin, LOW);
}

