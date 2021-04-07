#include <AccelStepper.h>

// Serial communication variables

const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;
char axis[buffSize] = {0};
char direct[buffSize] = {0};

uint8_t STEP_X = 2;

uint8_t STEP_Y = 3;

uint8_t STEP_Z = 4;

 

uint8_t DIR_X = 5;

uint8_t DIR_Y = 6;

uint8_t DIR_Z = 7;

 
int MaxxSpeed = 800;
int MaxzSpeed = 700;
int MaxySpeed = 700;

int Accel = 6000;

uint8_t EnablePin = 8;

 

AccelStepper x_axis(AccelStepper::DRIVER, STEP_X, DIR_X);

AccelStepper y_axis(AccelStepper::DRIVER, STEP_Y, DIR_Y);

AccelStepper z_axis(AccelStepper::DRIVER, STEP_Z, DIR_Z);


int LimX = 9;

int LimY = 10;

int LimZ = 11;

 

void setup() {
  
  
  x_axis.setMaxSpeed(MaxxSpeed);

  x_axis.setAcceleration(Accel);

  x_axis.moveTo(0);

 

  y_axis.setMaxSpeed(MaxySpeed);

  y_axis.setAcceleration(Accel);

  y_axis.moveTo(0);

 

  z_axis.setMaxSpeed(MaxzSpeed);

  z_axis.setAcceleration(Accel);

  z_axis.moveTo(0);

 

  x_axis.setEnablePin(EnablePin);
  y_axis.setEnablePin(EnablePin);
  z_axis.setEnablePin(EnablePin);

  digitalWrite(EnablePin, HIGH);

  pinMode(LimX, INPUT);  // x limit switch
  pinMode(LimY, INPUT); // y limit switch
  pinMode(LimZ, INPUT); // z limit switch

  
 /*
  * For the shield, connect the wires (from top to bottom) in order of:
  * Green
  * Gray/blue
  * Red
  * Yellow
  */
digitalWrite(EnablePin, LOW);  //motors on
// digitalWrite(EnablePin, HIGH);  //motors off


  // Limit switches are low when triggered
  
  Serial.begin(115200);  //115200
  Serial.print("Arduino is ready\r");
}


int i = 0;
bool stopped = false;
// String commandin;
int steps;
//String response;

void loop() {

    getDataFromPC();
    checklimits();
    select_movement();
    replyToPC();
   }



//===========================================

void movex()
    
{
  if (!stopped) {
  if (strcmp(direct, "-") == 0) {
    x_axis.move(steps); //negative direction switched in keep with tank coordinates
  }
  else {
    x_axis.move(steps*-1);
  }
  
  bool moved = true;
  while (digitalRead(LimX) != LOW && moved) {   
        moved = x_axis.run();  //returns true if it moved a step
  }
    if (digitalRead(LimX) == LOW) {
      int last;
      if (strcmp(direct, "-") == 0) {
        last = -1;
      }
      else {
        last = 1;
      }
      x_axis.move(last * 200);
      moved = true;
      while (moved) {
         moved = x_axis.run();
      }  
    }
  }
} //movex end

//=================

void movey()
{
  if (!stopped) {
    if (strcmp(direct, "-") == 0) {
    y_axis.move(steps); //The negative direction is switched in keep with tank coordinates 
    }
  else {
    y_axis.move(steps*-1);
  }
  
  bool moved = true;
  while (digitalRead(LimY) != LOW && moved) {   
        moved = y_axis.run();  //returns true if it moved a step
  }
    if (digitalRead(LimY) == LOW) {
      int last;
      if (strcmp(direct,"-") == 0) {
        last = -1;
      }
      else {
        last = 1;
      }
      y_axis.move(last * 200);
      moved = true;
      while (moved) {
         moved = y_axis.run();
      } 
      }
    }
  } //movey end
  

//=================

void movez()
{
  if (!stopped) {
   if (strcmp(direct, "-") == 0) {
    z_axis.move(steps*-1);
   }
  else {
    z_axis.move(steps);
  }
  
  bool moved = true;
  while (digitalRead(LimZ) != LOW && moved) {   
        moved = z_axis.run();  //returns true if it moved a step
  }
    if (digitalRead(LimZ) == LOW) {
      int last;
      if (strcmp(direct, "-") == 0) {
        last = 1;
      }
      else {
        last = -1;
      }
      z_axis.move(last * 200);
      moved = true;
      while (moved) {
         moved = z_axis.run();
      } 
    }
  }
}

//==========================================

void HomeMotors()
{
    digitalWrite(EnablePin, LOW);
    x_axis.move(100000);  //
    y_axis.move(10000);
    z_axis.move(-1000000);

    x_axis.run();
    y_axis.run();
    z_axis.run(); 
    
    
    bool xbool = true;
    bool ybool = true;
    bool zbool = true;
    do {      
          x_axis.run();
          y_axis.run();
          z_axis.run();
     
         if (digitalRead(LimX) == LOW) {
            xbool = false;
            x_axis.stop();  // Stop as fast as possible: sets new target         
         }
         if (digitalRead(LimY) == LOW) {
            ybool = false;
            y_axis.stop();  // Stop as fast as possible: sets new target
         }
         if (digitalRead(LimZ) == LOW) {
            zbool = false;
            z_axis.stop();  // Stop as fast as possible: sets new target      
         } 
    }
         while ((xbool || ybool) || zbool); 
         
            //step off limit switch
            x_axis.move(-500);
            y_axis.move(-500);
            z_axis.move(500);
            x_axis.runToPosition();
            y_axis.runToPosition();
            z_axis.runToPosition();
         

       //  digitalWrite(EnablePin, HIGH);
       
}
  

//==============================

void checklimits() {
   if (digitalRead(LimX) == LOW) // It's pushing on the x limit switch
           {
              stopped = true;
      }
   else if (digitalRead(LimY) == LOW) // It's pushing on the y limit switch
  {
    stopped = true;
  }
  else if (digitalRead(LimZ) == LOW) // it's pushing on the z limit switch
  {
    stopped = true;
  }
  else {
    stopped = false;
 }
}  //check limits end


//=================================
void getDataFromPC() {

    // receive data from PC and save it into inputBuffer
    
  if(Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant
      
    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();  
    }
    
    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) { 
      bytesRecvd = 0; 
      readInProgress = true;
    }
  }
}

//===============

void parseData() {

    // split the data into its parts
    
  char * strtokIndx; // this is used by strtok() as an index
  
  strtokIndx = strtok(inputBuffer,",");      // get the first part - the string
  strcpy(axis, strtokIndx); // 

  strtokIndx = strtok(NULL,",");      
  strcpy(direct, strtokIndx);
  
  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  steps = atoi(strtokIndx);     // convert this part to an integer
  
}

//----------------------------

void replyToPC() {

  if (newDataFromPC) {
    newDataFromPC = false;
   
    Serial.print(axis);
    Serial.print(direct);
    Serial.print(steps);
    Serial.println("\n");

  }
}

//==================

void select_movement() {  //Can modify this to call move functions
  if (newDataFromPC) {
   // this illustrates using different inputs to call different functions
  if (strcmp(axis, "x") == 0) {
      movex();
  }
  else if (strcmp(axis, "y") == 0) {
     movey();
  }
  else if (strcmp(axis, "z") == 0) {
    movez();
  }
  else if (strcmp(axis, "h") == 0) {
     HomeMotors();
    }
  }
  } // end select_movement
