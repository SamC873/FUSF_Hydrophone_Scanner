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

int xlim = 511; //mm
int ylim = 68;
int zlim = 170;

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

  pinMode(LimX, INPUT_PULLUP);  // x limit switch
  pinMode(LimY, INPUT_PULLUP); // y limit switch
  pinMode(LimZ, INPUT_PULLUP); // z limit switch

  
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
int ti = 0;
// String commandin;
int steps;
//String response;

void loop() {
/*if (digitalRead(LimX) || digitalRead(LimY) || digitalRead(LimZ) == 0){
    Serial.print(String(analogRead(LimX)));
    Serial.print('\n');
    Serial.print(String(analogRead(LimY)));
    Serial.print('\n');
    Serial.print(String(analogRead(LimZ)));
    Serial.print('\n');
    Serial.print("-");
    Serial.print('\n');
    }*/
    getDataFromPC();
    checklimits();
    select_movement();
    replyToPC();
   }



//===========================================

void movex()
{
//Serial.print(bool(stopped));
  //if (!stopped) {
    
  if (strcmp(direct, "-") == 0) {
    x_axis.move(steps*1); //negative direction switched in keep with tank coordinates
   // Serial.print("u moved in -x");
  }
  else {
    x_axis.move(steps*-1); //Note .move() sets the target position relative to current position
  }
  
  bool moved = true;
  while (digitalRead(LimX) != LOW && moved) {   
        moved = x_axis.run(); //Note .run() returns true if motor is still running to target position
        //Serial.print(moved);
  }
    if (digitalRead(LimX) == LOW) { // x limit switch hit
      int last;
      if (strcmp(direct, "-") == 0) { // -x limit switch hit
        last = -1; // step in + direction
        Serial.print("-x reached");
      }
      else {
        last = 1; // +x limit switch hit so step in - direction
      }
      moved = true;
              ti = 0;
              while (digitalRead(LimX) == LOW || moved) {
                 ti = ti + 1;
                Serial.print(ti);
                x_axis.move(last * 25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = x_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      
      stopped = false;
  //  }
  }
} //movex end

//=================

void movey()
//Last update 7/6/21
{
  //Serial.print(bool(stopped));
  //if (!stopped) {
    
  if (strcmp(direct, "-") == 0) {
    y_axis.move(steps*-1); //negative direction for tank coordinates
   // Serial.print("u moved in -y");
  }
  else {
    y_axis.move(steps*1); //Note .move() sets the target position relative to current position
  }
  
  bool moved = true;
  while (digitalRead(LimY) != LOW && moved) {   
        moved = y_axis.run();  //returns true if it moved a step
        //Serial.print(moved);
  }
    if (digitalRead(LimY) == LOW) { // y limit switch hit
      int last;
      if (strcmp(direct, "-") == 0) { // -y limit switch hit
        last = 1; // step in + direction
       // Serial.print("-y reached");
      }
      else {
        last = -1; // +y limit switch hit so step in - direction
      }
      moved = true;
              while (digitalRead(LimY) == LOW || moved) {
                y_axis.move(last * 25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = y_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      
      stopped = false;
  //  }
  }
  } //movey end
//=================

void movez()
{
   if (strcmp(direct, "-") == 0) {
    z_axis.move(steps*-1); //z axis has opposite of x and y if positive motion is away from motor
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
   
         moved = true;
              while (digitalRead(LimZ) == LOW || moved) {
                z_axis.move(last * 25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = z_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      stopped = false;    
  }
}//movez end

//==========================================

void HomeMotors()
{
  digitalWrite(EnablePin, LOW);
  // center x
    x_axis.move(10000000);    
    bool moved = true;  
      while (digitalRead(LimX) == HIGH && moved){
        moved = x_axis.run();
      }
      if (digitalRead(LimX) == LOW) {
        moved = true;
              while (digitalRead(LimX) == LOW || moved) {
                 
                x_axis.move(-25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = x_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
        stopped = true;
        moved = true;
                while (moved) {
                x_axis.move(-xlim/2*100); //moves to center of tank
                moved = true;
                stopped = false;
                while (moved) {
                    moved = x_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      } //end while loop which moved to center of tank

   //center y
    y_axis.move(-100000);    
    moved = true;  
      while (digitalRead(LimY) == HIGH && moved){
        moved = y_axis.run();
      }
      if (digitalRead(LimY) == LOW) {
        moved = true;
              while (digitalRead(LimY) == LOW || moved) {
                y_axis.move(25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = y_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
        stopped = true;
        moved = true;
                while (moved) {
                y_axis.move(ylim/2*100); //moves to center of tank
                moved = true;
                stopped = false;
                while (moved) {
                    moved = y_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      } //end while loop which moved to center of tank

    //center z
    z_axis.move(-1000000);    
    moved = true;  
      while (digitalRead(LimZ) == HIGH && moved){
        moved = z_axis.run();
      }
      if (digitalRead(LimZ) == LOW) {
        moved = true;
              while (digitalRead(LimZ) == LOW || moved) {
                z_axis.move(25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = z_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
        stopped = true;
        moved = true;
                while (moved) {
                z_axis.move(zlim/2*100); //moves to center of tank, note no negative due to signs along z axis
                moved = true;
                stopped = false;
                while (moved) {
                    moved = z_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      } //end while loop which moved to center of tank    
}//end HomeMotors()
 

//==============================

void checklimits() {
   if (digitalRead(LimX) == LOW) // It's pushing on the x limit switch
           {
              stopped = true;
              Serial.print("\nX Limit Pushed");
              // x limit switch hit
              int last;
              if (strcmp(direct, "-") == 0) { // -x limit switch hit
                  last = -1; // step in + direction
              }
              else {
              last = 1; // +x limit switch hit so step in - direction
              }
              bool moved = true;
              ti = 0;
              while (digitalRead(LimX) == LOW || moved) {
                ti = ti + 1;
                Serial.print(ti);
                x_axis.move(last * 25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = x_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      
      stopped = false;
           }
   else if (digitalRead(LimY) == LOW) // It's pushing on the y limit switch
  {
    stopped = true;
    Serial.print("\nY Limit Pushed");
              int last;
              if (strcmp(direct, "-") == 0) { // -y limit switch hit
                  last = 1; // step in + direction
              }
              else {
              last = -1; // +y limit switch hit so step in - direction
              }
              bool moved = true;
              while (digitalRead(LimY) == LOW || moved) {
                y_axis.move(last * 25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = y_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      
      stopped = false;
  }
  else if (digitalRead(LimZ) == LOW) // it's pushing on the z limit switch
  {
    stopped = true;
    Serial.print("\nZ Limit Pushed");
              int last;
              if (strcmp(direct, "-") == 0) { // -z limit switch hit
                  last = 1; // step in + direction
              }
              else {
              last = -1; // +z limit switch hit so step in - direction; NA in current system so need to watch bottom of tank
              }
              bool moved = true;
              while (digitalRead(LimZ) == LOW || moved) {
                z_axis.move(last * 25); //moves off limit switch in opposing direction in 250 um increments
                moved = true;
                while (moved) {
                    moved = z_axis.run(); //Note .run() returns true if motor is still running to target position
                  }  
                }
      
      stopped = false;
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
 // if (digitalRead(LimX) == HIGH && digitalRead(LimY) == HIGH && digitalRead(LimZ) == HIGH){
      Serial.print(axis);
      Serial.print(direct);
      Serial.print(steps);
      Serial.println("\n");
  }
  /* else { //A limit switch is activated requiring further instruction
      Serial.print(direct);
      Serial.print(axis);
      Serial.print(" limit reached");
   }*/
  //}
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
