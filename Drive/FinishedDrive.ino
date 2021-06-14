#include <SPI.h>
#include <math.h>
#include <Wire.h>
#include <INA219_WE.h>
#include "Mousecam.h"
#include "PID.h"

//PIN DEFINITIONS******************************************************
#define PIN_SS                                      10
#define PIN_MISO                                    12
#define PIN_MOSI                                    11
#define PIN_SCK                                     13
#define PIN_MOUSECAM_RESET                          8
#define PIN_MOUSECAM_CS                             7
//MOUSECAM VALUES******************************************************
#define ADNS3080_PIXELS_X                           30
#define ADNS3080_PIXELS_Y                           30
#define ADNS3080_PRODUCT_ID                         0x00
#define ADNS3080_REVISION_ID                        0x01
#define ADNS3080_MOTION                             0x02
#define ADNS3080_DELTA_X                            0x03
#define ADNS3080_DELTA_Y                            0x04
#define ADNS3080_SQUAL                              0x05
#define ADNS3080_PIXEL_SUM                          0x06
#define ADNS3080_MAXIMUM_PIXEL                      0x07
#define ADNS3080_CONFIGURATION_BITS                 0x0a
#define ADNS3080_EXTENDED_CONFIG                    0x0b
#define ADNS3080_DATA_OUT_LOWER                     0x0c
#define ADNS3080_DATA_OUT_UPPER                     0x0d
#define ADNS3080_SHUTTER_LOWER                      0x0e
#define ADNS3080_SHUTTER_UPPER                      0x0f
#define ADNS3080_FRAME_PERIOD_LOWER                 0x10
#define ADNS3080_FRAME_PERIOD_UPPER                 0x11
#define ADNS3080_MOTION_CLEAR                       0x12
#define ADNS3080_FRAME_CAPTURE                      0x13
#define ADNS3080_SROM_ENABLE                        0x14
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER       0x19
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER       0x1a
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER       0x1b
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER       0x1c
#define ADNS3080_SHUTTER_MAX_BOUND_LOWER            0x1e
#define ADNS3080_SHUTTER_MAX_BOUND_UPPER            0x1e
#define ADNS3080_SROM_ID                            0x1f
#define ADNS3080_OBSERVATION                        0x3d
#define ADNS3080_INVERSE_PRODUCT_ID                 0x3f
#define ADNS3080_PIXEL_BURST                        0x40
#define ADNS3080_MOTION_BURST                       0x50
#define ADNS3080_SROM_LOAD                          0x60
#define ADNS3080_PRODUCT_ID_VAL                     0x17
//MOVEMENT AND POSITION DATA*****************************************************
double theta = 1.57;                                //Angle the rover is facing
double dtheta = 0;                                  //Change in the rovers angle
double ttheta = 0;                                  //Theta to be transmitted to control
double dxmm = 0;                                    //Change in X (mm)
double dymm = 0;                                    //Change in Y (mm)
double forvel = 0;                                  //(Forward velocity)
double angvel = 0;                                  //Angular velocity
double xmm = 0;                                     //Current X position
double ymm = 0;                                     //Current Y position
byte Instructions[2] = {255, 255};                  //Instructions from command
int index = 0;                                      //Used for reading instructions from command
float RequestFlag = 0;                              //Has position been requested
bool Stationary = 1;                                //Is the rover stationary

//SMPS VALUES*******************************************************************
float closed_loop;                                                      // Duty Cycle
float vpd,vb,vref,iL,dutyref,current_mA;                                // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2,sensorValue3;       // ADC sample values declaration
float cv=0,oc=0; //Internal signals
float dt = 0.004; //250 Hz control frequency.
unsigned long LoopTime = 0; //Used to exectute void loop at 312.5Hz
float current_limit = 3.0;  //Current limit
PID PIDI(0, 1, 0.2, 39.4, 0); //PID object for current control
PID PIDV(0, 4, 0.2, 15.78, 0); //PID object for voltage control

//POSITION DATA FROM MOUSECAM****************************************************
struct MD                                           //Struct to store values recieved from mousecam
{
 byte motion;
 char dx, dy;
 byte squal;
 word shutter;
 byte max_pix;
};

volatile byte movementflag=0;         
volatile int xydat[2];
int tdistance = 0;

byte frame[ADNS3080_PIXELS_X * ADNS3080_PIXELS_Y];

//MOTOR PINS*******************************************************************************************************
int DIRL = 20;                                      //Defining left direction pin
int DIRR = 21;                                      //Defining right direction pin
int pwmr = 5;                                       //Pin to control right wheel speed using PWM
int pwml = 9;                                       //Pin to control left wheel speed using PWM

//ANGULAR RATE PID VALUES***********************************************************************
double contangvel = 0;                              //The control angular velocity
double angvelPID = 0;                               //PID output
PID PIDW(-255, 255, 1500, 10, 10);                  //PID object to control angular velocity

//ANGLE PID VALUES*********************************************************************************************
PID PIDT(-1.2, 1.2, 1, 10, 0);                      //PID object to control theta
bool ThetaFlag = 0;                                 //Used to decide when to turn
double instang = 0;                                 //Instructed angle

//POSITION PID VALUES****************************************************************************************************
bool startflag = 0;                                 //Is the rover already moving
float initx = 0;                                    //Start X
float inity = 0;                                    //Start Y
float dxP = 0;                                     //Displacement from desired position
float dyP = 0;                                     
float desx = 0;                                     //Desired positon
float desy = 0;
float xT = 0;                                       //Distance travelled from position at start of driving function
float yT = 0;                                       
float _destheta = 0;                                //Required angle
float d = 0;                                        //Distance from start
double contvel = 0;                                 //Control velocity
double p = 0;                                       //Distance from desired position
bool PosFlag = 0;                                   //Used to decide when to move
double instpos = 0;                                 //Instructed distance to move
PID PIDS(-255, 255, 1000, 500, 0);                  //PID object to control movement

//SETUP MOUSECAM OBJECT***************************************************************
Mousecam Camera(PIN_SS, PIN_MISO, PIN_MOSI, PIN_SCK, PIN_MOUSECAM_RESET, PIN_MOUSECAM_CS, ADNS3080_PIXELS_X, ADNS3080_PIXELS_Y, ADNS3080_PRODUCT_ID, 
           ADNS3080_REVISION_ID, ADNS3080_MOTION, ADNS3080_DELTA_X, ADNS3080_DELTA_Y, ADNS3080_SQUAL, ADNS3080_PIXEL_SUM, ADNS3080_MAXIMUM_PIXEL, 
           ADNS3080_CONFIGURATION_BITS, ADNS3080_EXTENDED_CONFIG, ADNS3080_DATA_OUT_LOWER, ADNS3080_DATA_OUT_UPPER, ADNS3080_SHUTTER_LOWER, ADNS3080_SHUTTER_UPPER,
           ADNS3080_FRAME_PERIOD_LOWER, ADNS3080_FRAME_PERIOD_UPPER, ADNS3080_MOTION_CLEAR, ADNS3080_FRAME_CAPTURE, ADNS3080_SROM_ENABLE, ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER,
           ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER, ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER, ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER, ADNS3080_SHUTTER_MAX_BOUND_LOWER, ADNS3080_SHUTTER_MAX_BOUND_UPPER,
           ADNS3080_SROM_ID, ADNS3080_OBSERVATION, ADNS3080_INVERSE_PRODUCT_ID, ADNS3080_PIXEL_BURST, ADNS3080_MOTION_BURST, ADNS3080_SROM_LOAD, ADNS3080_PRODUCT_ID_VAL);


//SMPS FUNCTIONS*************************************************************************************
INA219_WE ina219;                                   // This is the instantiation of the library for the current sensor

void sampling(){
  // Make the initial sampling operations for the circuit measurements
  sensorValue0 = analogRead(A0);                    //Sample Vb
  sensorValue2 = analogRead(A2);                    //Sample Vref
  sensorValue3 = analogRead(A3);                    //Sample Vpd
  current_mA = ina219.getCurrent_mA();              //Sample the inductor current (via the sensor chip)

  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023 
  // representing a voltage between 0 and the analogue reference which is 4.096V
  
  vb = sensorValue0 * (4.096 / 1023.0);             // Convert the Vb sensor reading to volts
  vref = sensorValue2 * (4.096 / 1023.0);           // Convert the Vref sensor reading to volts
  vpd = sensorValue3 * (4.096 / 1023.0);            // Convert the Vpd sensor reading to volts
  
  iL = current_mA/1000.0;                           // The inductor current is in mA from the sensor so we need to convert to amps.
  
}

float saturation( float sat_input, float uplim, float lowlim){      // Saturation function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}

void pwm_modulate(float pwm_input){                 // PWM of SMPS function
  analogWrite(6,(int)(255-pwm_input*255)); 
}
  
//MOTION CONTROL FUNCTIONS********************************************************************
void anglecontrol(float destheta){                  //Function to control angle
  contangvel = PIDT.Control(destheta, theta, dt);   //PID to calculate the required angular velocity
  angvelPID = PIDW.Control(contangvel, angvel, dt); //PID to calculate the PID
  analogWrite(pwmr, abs(angvelPID));                //Write PID to motors
  analogWrite(pwml, abs(angvelPID));
if(contangvel > 0){                                 //Control direction of wheels
  digitalWrite(DIRR, HIGH);
  digitalWrite(DIRL, HIGH);
  }
else{  
  digitalWrite(DIRR, LOW);
  digitalWrite(DIRL, LOW);
  }
if(destheta - theta < 0.04 && destheta - theta > -0.04 ){         //Stops when theta is within 1 degree
  analogWrite(pwmr, 0);              
  analogWrite(pwml, 0);
  ThetaFlag = 0;                                                    //Resets instructions
  PosFlag = 0;
  Instructions[0] = 255;
  }
}

void poscontrol(float s){                             //Function to control movement
    if(startflag == 0){                               //Calculates the desired end coordinates from the distance it should move
      desx = s * cos(theta) + xmm;
      desy = s * sin(theta) + ymm;
      initx = xmm;                                    //Records start position
      inity = ymm;
      startflag = 1;
    }
    dxP = desx - xmm;                                 //Distance from end coordinates
    dyP = desy - ymm;
    xT = xmm - initx;                                 //Distance from beginning
    yT = ymm - inity;
    if(dxP > 0){                                      //Angle to point at the end coordinates
      _destheta = atan(dyP/dxP);                      
    }else{
      _destheta = 3.14159 + atan(dyP/dxP);  
    }
    if(s < 0){
      _destheta = _destheta + 3.14159;
    }
    if (_destheta > 3.142){                           //Keep theta in the range -pi to pi
      _destheta = _destheta - 6.283185307;
    }
    if (_destheta < -3.142){
      _destheta = _destheta + 6.283185307;
    }
    p = sqrt(dxP * dxP + dyP * dyP);                  //Displacement from end coordinates
    d = sqrt(xT * xT + yT * yT);                      //Displacement from start
    if(theta - _destheta < 0.16 && theta - _destheta > -0.16){    //Decides if angle correction is required
      if(s > 0){                                                  //Drive forwards
        contvel = PIDS.Control(s, d, dt);
        digitalWrite(DIRR, HIGH);
        digitalWrite(DIRL, LOW);
      }else{                                                      //Drive backwards
        contvel = PIDS.Control(s, d, dt);
        digitalWrite(DIRR, LOW);
        digitalWrite(DIRL, HIGH);
      }
    }else{
        contangvel = PIDT.Control(_destheta, theta, dt);
        contvel = PIDW.Control(contangvel, angvel, dt);
        analogWrite(pwmr, abs(angvelPID));
        analogWrite(pwml, abs(angvelPID));
        if(contangvel > 0){  
        digitalWrite(DIRR, HIGH);
        digitalWrite(DIRL, HIGH);
        }else{  
        digitalWrite(DIRR, LOW);
        digitalWrite(DIRL, LOW);
        }
    }
    analogWrite(pwmr, abs(contvel));                  //Controls motor speed
    analogWrite(pwml, abs(contvel));
    if(p < 5){                                        //Stop if within 10mm of the end coordinates
      analogWrite(pwmr, 0);
      analogWrite(pwml, 0); 
      PosFlag = 0;                                    //Resets instructions                                               
      ThetaFlag = 0;
      startflag = 0;
      Instructions[0] = 255;
    }
}

//SETUP**********************************************************************************************************************
void setup() {

  pinMode(PIN_SS,OUTPUT);                             //Setup pins to communicate with mousecam              
  pinMode(PIN_MISO,INPUT);
  pinMode(PIN_MOSI,OUTPUT);
  pinMode(PIN_SCK,OUTPUT);

  pinMode(4, OUTPUT);
  analogReference(EXTERNAL);                          // We are using an external analogue reference for the ADC for the SMPS

  // TimerB0 initialization for PWM output
  
  pinMode(6, OUTPUT);
  TCB0.CTRLA=TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;   //62.5kHz PWM for SMPS control
  analogWrite(6,120); 

  Wire.begin();                                       // We need this for the i2c comms for the current sensor
  ina219.init();                                      // This initiates the current sensor
  Wire.setClock(700000);                              //Set the comms speed for i2c

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  
  Serial.begin(115200);                               //Begin serial, used for testing
  Serial1.begin(115200);                              //Begin serial1, used for communication with control

  pinMode(DIRR, OUTPUT);                              //Setup motor pins
  pinMode(DIRL, OUTPUT);
  pinMode(pwmr, OUTPUT);
  pinMode(pwml, OUTPUT);
  
  if(Camera.mousecam_init()==-1)                      //Initialises mousecam
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  } 
  
}

//LOOP******************************************************************************************************************************

void loop() {
  if(millis() > LoopTime) {                           //This loop will run every 4mS
    digitalWrite(4, HIGH);
    sampling();                                       // Sample all of the measurements    
    // The closed loop path has a voltage controller cascaded with a current controller. The voltage controller
    // creates a current demand based upon the voltage error. This demand is saturated to give current limiting.
    // The current loop then gives a duty cycle demand based upon the error between demanded current and measured current
    cv = PIDV.Control(vref, vb, dt);                  //Voltage pid
    cv=saturation(cv, current_limit, 0);              //current demand saturation
    closed_loop=PIDI.Control(cv, iL, dt);             //Current pid
    closed_loop=saturation(closed_loop,0.99,0.01);    //Duty_cycle saturation
    pwm_modulate(closed_loop);                        //Pwm modulation
    LoopTime = LoopTime + dt * 1000;                  //Time to execute next loop
    
    int val = Camera.mousecam_read_reg(ADNS3080_PIXEL_SUM);                                             //Read mouscam
    MD md;                                                                                              //Create struct to store mouscam data
    Camera.mousecam_read_motion(&md.motion, &md.dx, &md.dy, &md.squal, &md.shutter, &md.max_pix);       //Read motion data from mouscam
    
    dxmm = 25.4 * md.dx / 400;                        //Calculate change in X, converting counts per inch to mm
    dymm = 25.4 * md.dy / 400;                        //Calculate change in Y, converting counts per inch to mm

    dtheta = atan(dxmm/(150 - dymm));                 //Calcualte change in angle
    theta = theta + dtheta;
    if (theta > 6.283){                               //Keep theta in the range -2pi to 2pi
      theta = theta - 6.283;
    }
    if (theta < -6.283){
      theta = theta + 6.283;
    }
    
    angvel = dtheta / dt;                             //Calculate angular velocity
    xmm = xmm + (dymm * cos(theta));                  //Calculate change to X coordinate
    ymm = ymm + (dymm * sin(theta));                  //Calculate change to Y coordinate

    if(PosFlag == 0 && ThetaFlag == 0){
      Stationary = 1;
    }else{
      Stationary = 0;
    }
    
    index = 0;                                        //Read instructions from Serial1
    while(Serial1.available()){
      Instructions[index] = Serial1.read();
      if(Instructions[index] == 252){                 //If the position is requested, do not add to Instructions
        RequestFlag = 1;
      }else{
        index++;
      }
    }
    
    if(Instructions[0] == 255){                       //If Instructions[0] = 255, the stop instruction has been called
      PosFlag = 0;
      ThetaFlag = 0;
      startflag = 0;
      analogWrite(pwmr, 0);
      analogWrite(pwml, 0);
    }
    
    if(Instructions[0] == 253 && Stationary == 1){    //If Instructions[0] = 253 the turn instruction has been called
      ThetaFlag = 1;
      instang = (Instructions[1] - 120) * 6.283 / 240;
    }
    
    if(Instructions[0] == 254 && Stationary == 1){    //If Instructions[0] = 254, the move forward instruction has been called
      PosFlag = 1;
      instpos = (Instructions[1] - 120) * 10;
    }

    if(RequestFlag == 1){                             //If a byte with value 252 is recieved, the rovers coordinates have been requested
      ttheta = theta;
      if (ttheta > 3.1415){                           //Keep ttheta in the range -pi to pi
        ttheta = ttheta - 6.283;
      }
      if (theta < -3.1415){
        theta = theta + 6.283;
      }
      Serial1.write(int(round(xmm/10) + 127));                      //Send coordinates in cm
      Serial1.write(int(round(ymm/10) + 127));
      Serial1.write(int((ttheta * 255 / 6.283) + 127));             //Send theta
      Serial1.write(int(Stationary));
      RequestFlag = 0;
    }
       
    if(ThetaFlag == 1 && PosFlag == 0){                             //If ThetaFlag = 1, the rover should control its angle
      anglecontrol(instang);
    }
    
    if(PosFlag == 1 && ThetaFlag == 0){                             //If PosFlag = 1, the rover should control its position
      poscontrol(instpos);
    }
    digitalWrite(4, LOW);
  }
}  
