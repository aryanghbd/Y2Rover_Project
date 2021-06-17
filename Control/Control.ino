#include <WiFi.h> /* WiFi library needed for connecting to WiFi on ESP32 */
#include <PubSubClient.h> /* PubSubClient needed for MQTT based API */
#include <math.h>
#include <HardwareSerial.h>  /* HardwareSerial needed to assign relavant pins on the board for UART communication */
const char* ssid = "YOUR SSID";
const char* pass = "YOUR NETWORK PASSWORD"; /* Required WiFi PARAMETERS to connect to WiFi */
const char* mqtt_user = "hs2119";
const char* mqttserver = "18.117.97.5";
const char* mqtt_pass = "marsrover";
const int mqttPort = 1883; /* Establishing MQTT connection parammeters */
const float Pi = 3.14159;
#define RXD2 16 /* Need these two UART pins for Drive, pins 8 and 9 respectively */
#define TXD2 17
#define RXD1 21 /* Need these two UART pins for Vision */
#define TXD1 22 
WiFiClient ESPClient; /* Creating WiFiClient to use the full WIFI API */
PubSubClient client(ESPClient); /* Creates pub */
int currentcoordinates[2]; /* A global array that is updated whenever the 'getPositionData() void function is called, so that we always have the rover's current position stored locally */
int waypointcoordinates[] = {0, 0}; /* Since waypointing and autopiloting call sequential instructions, and the drive subsystem ignores them if timed during an instruction, waypointing and autopilot must be done within void loop, meaning that waypoint coordinates must be stored globally too */
int waypointdifferences[] = {0, 0}; /* Waypoint difference array stores the difference between the target X and Y value vs the rover's current X and Y value in currentcoordinates */
float desiredangle = 0; /* When waypointing, we want to turn towards the point we are driving to, this global variable stores the desired angle, calculated in function, so it can be used in void loop */
float currentangle = 0; /* Drive subsystem sends its position as well as the angle measured against a local reference, like currentcoordinates, we can store this value locally for comparison and use in other functions in real time */
bool isBusy = true; /* This variable is active LOW, sent from Drive, when the value is 1, this indicates that the rover is stationary and ready to act on a new instruction, when 0 it means that the rover is currently doing an instruction, used for debugging. */
bool stopped = false; /* Variable called and used for debugging, sent when the stoprover method is called to indicate the rover has been forceibly stopped. */
bool waypointstart = false; /* Flag set high when the waypoint instruction is called, in void loop every 250ms this flag is checked against before acting */
bool autopilotstart = false; /* Similar flag to waypointstart flag but for autopilot instructions */
int autopilotcycles = 0; /* Since autopilot is done in void loop, having the number of autopilot cycles stored globally allows this to be referenced */
int autopilotcounter = 0; /* Global counter for how many cycles has elapsed during an autopilot instruction */
int range = 100; /* Maximum range of the autopilot instruction, which goes to nearby coordinates, this has been made 100 as the testing area is 1m x 1m */
byte visiondistance1[12] = {0}; /* A byte array that stores the 12 bytes that come through every cycle from vision, giving the bounding boxes for 5 diff colours with the last 2 bytes being 0.*/
int colourarray[5] = {0}; /* Array to store processed bounding box distance data */
int proximity; /* Value to store the distance to the main colour of the object */
int colour; /* Variable that changes from 0 to 4 depending on the colour, green black, blue pink orange*/
int objectcoordinates[2] = {0}; /* When an object is seen, and a colour and proximity is found, the object's coordinates are approximated by taking the proximity to the object and comparing with the rover's position and angle already */
bool autopilotflag = false; /* If an automatic stop is issued, this flag is called when it is done in the middle of an autonomous instruction */
/* Creating an instance of PubSubClient which will enable MQTT transfers */
static esp_err_t esp_wifi_set_ps(WIFI_PS_MODEM); /* Power Save Mode is ON by default, however the user can adjust this as they wish by issuing the instruction 'POWERSAVE ON/OFF' */
const char* topic = "instruction"; /* This is the topic that our endpoint, the ESP32 is looking out for and listening for*/
bool oflag = false;
bool gflag = false;
bool pflag = false;
bool bflag = false;
  
void setup_wifi() {
  delay(10);
  /*Connect to WiFi first*/
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass); /* Issue connection to the main network */
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); /* Continue doing ... until the WiFi status is connected */
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

void movedistance(int distance) {
  stopped = false; /* Set stopped flag to false to enable the movement */
  Serial.println();
  Serial.print("MOVING FORWARD, Amount: "); /* Print transmission data to Serial port on main PC, for debugging purposes, if energy was present we would just delete the serial print lines as they are already on the Command interface.*/
  Serial.print(distance - 120); /* Drive subsystem works by taking a number between 0 to 240, numbers between 0 to 120 will cause the rover to reverse by 120cm or 0cm respectively, numbers from 120 to 240 will move 0cm-120cm forward. */
  Serial.print(" centimetres.");
  Serial.println();
  Serial.print("Issuing the sequence: ");
  Serial.print(254, BIN); 
  Serial.print(distance, BIN);
  Serial.print(" via UART.."); /* The Drive subsystem works by sending differing byte sequences depending on instruction. So for a FORWARD instruction, it recognises this as being FORWARD by sending the byte 254, followed by the number between 0 to 240, however since this is to move forward, we must offset this in the main function.
  of the amount that the rover will move in CM. */
  String feedback = "Moving FORWARD by " + (String)(distance - 120) + " centimetres..";
  client.publish("feedback", (char*)feedback.c_str()); /* Publish serial debug string to the MQTT broker so Command can see the result of their instruction */
  Serial2.write(254);
  Serial2.write(distance); /* Serial2 is the UART channel established for Drive, it is a full duplex channel which means information can flow in both directions simultaneously */
}

void rotateamount(int amount) { /* Method that rotates the rover, Drive turns to a given angle position when transmitting the byte 253, followed by the amount, between 0 to 240, rotating pi to -pi radians */
  stopped = false;
  Serial.println();
  float angle = 2 * Pi / 240 * int(amount); /* Same range of 0 to 240 from before, however the number represents the angle position the rover will turn to, characterised by this formula */
  Serial.print("TURNING, Amount: ");
  Serial.print(angle);
  Serial.print(" radians clockwise.");
  Serial.println();
  Serial.print("Issuing the sequence: ");
  Serial.print(253, BIN);
  Serial.print(amount, BIN);
  Serial.print(" via UART..");
  String feedback = "Turning by " + (String)(amount) + " radians clockwise..";
  client.publish("feedback", (char*)feedback.c_str());
  Serial2.write(253);
  Serial2.write(amount);
}

int getX() {
  return currentcoordinates[0]; /* Useful method which fetches the X pos of the rover from the currentcoordinates array.*/
}

int getY() {
  return currentcoordinates[1];
}

float getDelay(float distance) {
  return 1000 * (distance / 3); /* Instructions need to be scaled based on the time of delay, so a function was made which uses millis but delays until this time, scaled by the distance needed to travel when considering the rover's average speed, *1000 to get in seconds, to ensure no instruction conflicts. */
}

/* I decided to keep this function underneath in comments to show how waypoint was initially developed, and why this had to be scrapped in lieu of a real-time equivalent in void loop */

//void waypoint(String coordinate) {
//  if (currentangle != 1.57) {
//    rotateamount(180);
//    delay(4000);
//    /*If the angle is different return to starting position where the thing is 1.55*/
//  }
//  /* Reset angle to the default angle, 1.35 radians, check this  */
//  String targetX = "";
//  String targetY = "";
//  for (int i = 0; i < coordinate.length(); i++) {
//    if (coordinate[i] == ',') {
//      break;
//    }
//    else {
//      targetX = targetX + coordinate[i];
//    }
//  }
//  int secondindex = coordinate.indexOf(',') + 1;
//  for (int j = secondindex; j < coordinate.length(); j++) {
//    targetY = targetY + coordinate[j];
//  }
//  int X, Y;
//  X = targetX.toInt();
//  Y = targetY.toInt();
//  float xreference, yreference;
//  float xdiff, ydiff;
//  xreference = currentcoordinates[0];
//  yreference = currentcoordinates[1];
//  Serial.println();
//  Serial.print("Driving to point - X: ");
//  Serial.print(X);
//  Serial.print(",");
//  Serial.print("Y: ");
//  Serial.print(Y);
//  String feedback = "Driving to point: " + coordinate;
//  client.publish("feedback", (char*)feedback.c_str());
//  xdiff = X - xreference;
//  ydiff = Y - yreference;
//  float angle = 0;
//  if (xdiff > 0) {
//    angle = atan(ydiff / xdiff) * (240 / 6.283);
//  }
//  else if (xdiff < 0) {
//    angle = (atan(ydiff / xdiff) + Pi);
//    if (angle > (Pi)) {
//      angle = angle - (2 * Pi);
//    }
//    else if (angle < (-Pi)) {
//      angle = angle + (2 * Pi);
//    }
//    angle = angle * (240 / 6.283);
//  }
//  else {
//    Serial.println();
//    Serial.print("Not a number");
//    client.publish("feedback", "Error: NAN");
//  }
//  /*  if (ydiff > 0) { /* Target angle is generally forward from the rover */
//  /* if(xdiff > 0) { /* Target angle is generally to the right of the rover */
//  /* Fetches angle we have to turn to */
//  /* Rotate amount angle, do we include delay here too?* 90 - angle*/
//  rotateamount(int(angle) + 120);
//  delay(5000);
//  float newamount = sqrt(sq(ydiff) + sq(xdiff)) + 120; /* Fetches hypotenuse to create shortest distance */
//  Serial.println();
//  movedistance(int(newamount));
//  delay(getDelay(newamount - 120));
//}

/* This function actually did work, but the inefficiency came from the fact that the function used the delay function very liberally, which is a cardinal sin in Arduino, especially for periods of time longer than a few ms, this is because the delay function effectively
halts the entire code for that length of time, meaning that there would be no coordinates or vision usage in the time taken, this only affects autonomous instructions but a way to be able to fetch data whiile performing would be to use waypointing with millis as a means to
compare time against when the ESP was first powered on, this can be done in void loop, meaning that to make this work it had to be rewritten from scratch. */

/* Brief summary of how waypoint works: The function gets the desired X,Y coordinates, and using the rover's current X,Y coordinates subtends it to a right angled triangle, we rotate the required angle formed by the triangle to align it to the desired point, then move it by the magnitude
of the hypotenuse to reach the point itself. It works best when the rover is aligned to it's initial startpoint beforehand, so everytime it is called we align back to be straight on before performing the other operations*/

void waypoint(String coordinate) { /* New and improved waypointing code, which sets global variables which are then called in void loop in order to use millis as a replacement for delay, and get data simultaneously as it moved. */
  waypointstart = true; /* Sets checkpoint flag to true */
  String targetX = "";
  String targetY = "";
  for (int i = 0; i < coordinate.length(); i++) {
    if (coordinate[i] == ',') {
      break;
    }
    else {
      targetX = targetX + coordinate[i]; /* Forming the X and Y targets in th is way, in order to avoid overallocation with the String constructor on Arduino. */
    }
  }
  int secondindex = coordinate.indexOf(',') + 1;
  for (int j = secondindex; j < coordinate.length(); j++) {
    targetY = targetY + coordinate[j];
  }
  int X, Y;
  X = targetX.toInt(); /* Convert parsed target to integers */
  Y = targetY.toInt();
  waypointcoordinates[0] = X; /* Setting global target coordinates array to parsed coordinates, these are replaced if this function is called again. */
  waypointcoordinates[1] = Y;
  waypointdifferences[0] = X - currentcoordinates[0]; /* This gathers difference between target and reference by substracting from current position. */
  waypointdifferences[1] = Y - currentcoordinates[1];
  if (waypointdifferences[0] > 0) { /* If difference in X is positive, we are in either the top right or bottom right quadrant on an X-Y plane, which means we calculate the angle in this way, offset by Drive's angle calculation. */
    desiredangle = atan(waypointdifferences[1] / waypointdifferences[0]) * (240 / 6.283);
  }
  else if (waypointdifferences[0] < 0) { /* If the difference in X is negative, we are in the bottom left or top left quadrants on the X-Y plane, meaning we have to offset by Pi and ensure that the desired angle is in the range Drive accepts. */
    desiredangle = (atan(waypointdifferences[1] / waypointdifferences[0]) + Pi);
    if (desiredangle > (Pi)) {
      desiredangle = desiredangle - (2 * Pi);
    }
    else if (desiredangle < (-Pi)) {
      desiredangle = desiredangle + (2 * Pi);
    }
    desiredangle = desiredangle * (240 / 6.283);
  }
  else { /* If diferences do not correspond. */
    Serial.println();
    Serial.print("Not a number");
    client.publish("feedback", "Error: NAN");
  }
}

void stoprover() { /* Drive stops the rover manually by sending a byte of constant 255. */
  Serial.println();
  Serial.print("STOPPING ROVER... ");
  client.publish("feedback", "STOPPING ROVER...");
  Serial2.write(255);
}

void reverse(int distance) { /* Reversing uses the same functionality as the movedistance function, but we do the offsetting in the method instead, REVERSE 10 means reverse 10cm, but Drive recognises this as 110, meaning that reversals must be subtracted from 120.*/
  Serial.println();
  Serial.print("REVERSING, Amount: ");
  Serial.print(distance);
  Serial.print(" centimetres.");
  Serial.println();
  Serial.print("Issuing the sequence: ");
  Serial.print(254, BIN);
  Serial.print((120 - distance), BIN);
  Serial.print(" via UART.."); /* The Drive subsystem works by sending differing byte sequences depending on instruction. So for a FORWARD instruction, it recognises this as being FORWARD by sending the byte 1, followed by the number between 0 to 255
  of the amount that the rover will move in CM. */
  String feedback = "Reversing " + (String)(distance) + " centimetres...";
  client.publish("feedback", (char*)(feedback).c_str());
  Serial2.write(254);
  Serial2.write(120 - distance);
}

void powersave(String status) { /* Sets WIFI powersave mode on the ESP32, allowing user to tailor seasmlessness vs power consumption, a good addition which may be even more useful if Energy was also integrated. */
  if (status[1] == 'N') {
    static esp_err_t esp_wifi_set_ps(WIFI_PS_MODEM); /* If the status is ON or OFF, change powersave mode accordingly and give info to the debug */
    Serial.print("WiFi Power Saving Mode is now On.");
    client.publish("feedback", "WiFi Power Saving Mode is now On.");
  }
  else if (status[2] == 'F') {
    static esp_err_t esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.print("WiFi Power Saving Mode is now Off.");
    client.publish("feedback", "WiFi Power Saving Mode is now Off.");
  }
  else {
    Serial.print("Error: NOT A VALID POWERSAVE MODE. Please select between ON or OFF"); /* Throw an error if the mode specified is not valid. */
    client.publish("feedback", "Error: NOT A VALID POWERSAVE MODE. Please select between ON or OFF");
    Serial.println();
    Serial.print(status);
  }
}

void displayhelp() { /* Displays a list of commands on Serial and MQTT.*/
  Serial.println();
  Serial.print("SELECT FROM THE FOLLOWING COMMANDS:");
  Serial.println();
  Serial.print("FORWARD X: MOVE FORWARD X CENTIMETRES.");
  Serial.println();
  Serial.print("TURN X: TURN TO POSITION X, ROTATING THE DRONE 2*PI/240 * X DEGREES CLOCKWISE.");
  Serial.println();
  Serial.print("STOP: STOP THE ROVER'S CURRENT ACTION.");
  Serial.println();
  Serial.print("POWERSAVE ON/OFF: TURN WIFI POWERSAVING MODE ON OR OFF.");
  Serial.println();
  Serial.print("WAYPOINT X,Y: MOVE TO POINT X,Y");
  Serial.println();
  Serial.print("AUTOPILOT X: AUTONOMOUSLY MOVE TO X COORDINATES IN THE AREA");
  client.publish("feedback", "FORWARD X: MOVES FORWARD X CM, TURN X: ROTATE TO POSITION X RELATIVE TO THE X AXIS, STOP: STOP THE ROVER, POWERSAVE ON/OFF: SWITCH WIFI POWERSAVING, WAYPOINT X,Y: MOVE TO POINT X,Y, AUTOPILOT X: AUTONOMOUSLY MOVE TO X COORDINATES IN THE SURROUNDING AREA");
}

//void test() {
  // movedistance(100);
  //delay(5000);
  //rotateamount(100);
  //delay(5000);
}
//void autopilot(int cycles) {
//  int i = 0;
//  Serial.println();
//  Serial.print("Take your hands off the keyboard, I'll take it from here..");
//  client.publish("feedback", "Take your hands off the keyboard, I'll take it from here..."); /* Publish the equivalent feedback to MQTT */
//  int range = 100;
//  int xreference, yreference;
//  xreference = currentcoordinates[0];
//  yreference = currentcoordinates[1]; /* By taking the difference between the reference position and the overall range, we can gather a field of values to randomly make the rover move around in */
//  while (i < cycles) { /* When autonomous mode is selected, the rover will move completely on its own until the user decides to intervene by issuing a STOP command */
//    int xnew = random(xreference + 1, range / 2);
//    int ynew = random(yreference + 1, range / 2);
//    String autocoord = "";
//    autocoord = String(xnew) + "," + String(ynew);
//    autocoord = autocoord.c_str();
//    Serial.println();
//    Serial.print("Chose to move to: ");
//    Serial.print(autocoord);
//    String feedback1 = "Chose to move to: " + autocoord;
//    client.publish("feedback", (char*)feedback1.c_str());
//    waypoint(autocoord);
//    int xend = getX();
//    int yend = getY();
//    Serial.print("I am now at: ");
//    Serial.print("X: ");
//    Serial.print(xend);
//    Serial.print(",");
//    Serial.print("Y: ");
//    Serial.print(yend);
//    String feedback2 = "Autonomously reached point- X: " + (String)xend + "," + "Y: " + (String)yend;
//    client.publish("feedback", (char*)feedback2.c_str());
//    i = i + 1;
//  }
//

void autopilot(int cycles) { /* Similarly to the autopilot function, this is acted upon in millis, but whenever this is called the initial coordinates must be set to 0. */
  autopilotstart = true; /* Set flags and global variables accordingly */
  autopilotcycles = cycles;
  autopilotcounter = 0;
}


void callback(char* topic, byte* payload, unsigned int length) { /* The PubSubClient MQTT API allows you to define how the system responds to a MQTT message, known as the 'callback' response, this is where we will react to incoming data and instructions */
  String convert = ""; /* Payload array is the 'message' we get on MQTT, by default this is a byte array, I want to turn this into a string to be able to manipulate, however by using the regular constructor, this means
  that I would end up converting hidden information such as the client info, which is normally hidden in the payload array, conversion to string must be done manually by concatonating the characters of the array with an empty string to generate our instruction.
*/
  String action = ""; /* The 'action' is the part of the instruction prior to spaces, which is what we are doing, the following actions are FORWARD, TURN, STOP and POWERSAVE. We need this to check what we need to do with the data*/
  for (int i = 0; i < length; i++) {
    convert = convert + (char)payload[i]; /* Concatonating the payload array to the string to form the converted string */
  }
  convert = convert.c_str();
  for (int j = 0; j < length; j++) {
    if ((char)payload[j] == ' ') {
      break;
    }
    else {
      action += (char)payload[j]; /* Concatonating the instruction to action until we detect a space */
    }
  }
  action = action.c_str(); /* Using the String class on Arduino longer than you really need to is a bad...bad idea. To avoid corrupting the memory, immediately cast these to c strings.*/
  int j = 0;
  switch ((char)payload[0]) { /* Switching based on the first character of the received array saves us from having to do further conversion or string copying, as opposed to a long tangled sequence of if/else statements */
    case 'F': /* If the first character is F, then this is probably going to be a forward instruction, using 'probably' as the user could potentially be typing a random word with the letter F at the start, error checking is a MUST to secure integrity */
      {
        Serial.println();
        if (action == "FORWARD" && convert.length() > 7) { /* Error Checking: If we have the first character as an F, DO NOT ACT unless the action is also forward, and the length of the instruction is exactly 11, the exact length of FORWARD XXX */
          int parameter = (convert.substring(8)).toInt(); /* Convert the last 3 digits to an integer to be able to convert and send */
          if (parameter < 0 || parameter > 120) {
            Serial.print("ERROR! Please declare a movement amount between 0 and 120cm."); /* Drive subsystem requires all movement amounts to be between 0 and 120cm, so check the parameter is within range first. */
            client.publish("feedback", "ERROR! Please declare a movement amount between 0 and 120cm.");
          }
          else {
            movedistance(parameter + 120); /* Calls main method */
          }
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction Detected. Did you mean 'FORWARD X'?"); /* If either the action isn't FORWARD or there are excess digits/less digits, throw an error.*/
          client.publish("feedback", "ERROR! Invalid Instruction Detected. Did you mean 'FORWARD X'?");
        }
      }
      break;
    case 'T':
      {
        if (action == "TURN" && convert.length() > 4) { /* Same logic applies from before with TURN instructions, just mandating that the length is 8 instead, TURN XXX */
          int parameter = (convert.substring(5)).toInt();
          if (parameter < 0 || parameter > 240) { /* Validation */
            Serial.println();
            Serial.print("ERROR! Please declare a turning position between 0 and 240.");
            client.publish("feedback", "ERROR! Please declare a turning position between 0 and 240.");
          }
          else {
            rotateamount(parameter);
          }
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction Detected. Did you mean 'TURN X'?"); /* Throw an error */
          client.publish("feedback", "ERROR! Invalid Instruction Detected. Did you mean 'TURN X'?");
        }
      }
      break;
    case 'P':
      {
        if (action == "POWERSAVE") {
          /* As said before, WIFI POWERSAVE mode can save some crucial time that may cause lagging between the network and the ESP, this defaults to ON, which means that in between instructions, when not using WIFI, it will turn off,
            this means that you effecitvely have to restart the WIFI systems to receive the next instruction which can waste time. The time difference cut is not overly impactful, however this was an extra feature added for the convenience of the user, who can turn powersave off
            if they want to prioritise lag reduction with excess battery consumption, or reduced battery consumption but with slightly longer roundtrip times.*/
          String status = (convert.substring(10)); /* This gathers the ON, or OFF part */
          Serial.println();
          powersave(status);
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction detected, did you mean 'POWERSAVE ON/OFF'?");
          client.publish("feedback", "ERROR! Invalid Instruction detected, did you mean 'POWERSAVE ON/OFF'?");
        }
      }
      break;
    case 'H':
      {
        if (action == "HELP") { /* Made a HELP command to give the user screen the list of possible commands */
          displayhelp();
          client.publish("feedback", "FORWARD X: MOVES FORWARD X CM, TURN X: ROTATE TO POSITION X RELATIVE TO THE X AXIS, STOP: STOP THE ROVER, POWERSAVE ON/OFF: SWITCH WIFI POWERSAVING, WAYPOINT X,Y: MOVE TO POINT X,Y, AUTOPILOT X: AUTONOMOUSLY MOVE TO X COORDINATES IN THE SURROUNDING AREA");
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction detected, did you mean 'HELP'?");
          client.publish("feedback", "ERROR! Invalid Instruction detected, did you mean 'HELP'?");
        }
      }
      break;
    case 'S':
      {
        if (action == "STOP") {
          stoprover();
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction detected, did you mean 'STOP'?");
          client.publish("feedback", "ERROR! Invalid Instruction detected, did you mean 'STOP'?");
        }
      }
      break;
    case 'W':
      {
        if (action == "WAYPOINT") {
          String status = convert.substring(9).c_str();
          waypoint(status); /* Instantiates method, validates similarly. */
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction detected, did you mean 'WAYPOINT X,Y'?");
          client.publish("feedback", "ERROR! Invalid Instruction detected, did you mean 'WAYPOINT X,Y'?");
        }
      }
      break;
    case 'A':
      {
        if (action == "AUTOPILOT") {
          String status = convert.substring(10).c_str();
          int cycles = status.toInt();
          autopilot(cycles);
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction detected. Did you mean 'AUTOPILOT X'?");
          client.publish("feedback", "ERROR! Invalid Instruction detected. Did you mean 'AUTOPILOT X'?");
        }
      }
      break;
    case 'R':
      {
        Serial.println();
        if (action == "REVERSE" && convert.length() > 7) { /* Validate reverse */
          int parameter = (convert.substring(8)).toInt(); /* Convert the last 3 digits to an integer to be able to convert and send */
          if (parameter < 0 || parameter > 120) {
            Serial.print("ERROR! Please declare a movement amount between 0 and 120cm."); /* Drive subsystem requires all movement amounts to be between 0 and 255cm, so check the parameter is within range first. */
            client.publish("feedback", "ERROR! Please declare a movement amount between 0 and 120cm.");
          }
          else {
            reverse(parameter);
          }
        }
        else {
          Serial.println();
          Serial.print("ERROR! Invalid Instruction Detected. Did you mean 'REVERSE X'?"); 
          client.publish("feedback", "ERROR! Invalid Instruction Detected. Did you mean 'REVERSE X'?");
        }
      }
      break;
    default: /* Default action to be taken if no action is recognised. */
      Serial.println();
      Serial.print("ERROR: Invalid instruction detected. Send 'HELP' for available instructions");
      client.publish("feedback", "ERROR: Invalid instruction detected. Send 'HELP' for available instructions");
      break;
  }

}

//void readEnergyData() { hypothetical energy reading function
//  int batterylife = Serial0.read(); //Presuming conversion was done on the Energy end.
//  String batterylifemqtt = String(batterylife);
//  client.publish("battery", (char*)batterylife.c_str();
//}

void reconnect() { /* Loop to establish MQTT connection, in the main loop, we will continuously check if the connection is still there, and if not this function will exexcute, i.e, at the start of the program */
  /*Loop until reconnected*/
  while (!client.connected()) {
    Serial.print("Attempting MQTT Connection..."); /* If not connected, attempt to connect. */
    /* MQTT requires an individual client ID in order to connect to the broker, this must be fixed */
    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX); /* Concatenating to put it into the correct format. */
    if (client.connect(clientID.c_str(), mqtt_user, mqtt_pass)) { /* API allows us to simply establish the connection by using the 3 global parameters defined at the top */
      Serial.println("Connected");
      /*When we connect to the server, publish a message*/
      client.publish("hello", "Mars Rover: ESP32 Controller successfully connected.");
      /*When connected sub to instructions topic that we will expect instructions from, we will not be subscribing to anything else, so we can keep the topic fixed.*/
      client.subscribe(topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println("Please try again in 5 seconds"); /* If the connection fails, give the network error code and reattempt in 5 seconds. */
      delay(5000);

    }
  }
}

void requestPosData() { /* Drive subsystem communicates the COORDINATES data by sending a request to the Arduino NANO to provide information. Transmitting the single byte 127 through UART will trigger the Arduino to send coordinates back through UART. */
  Serial2.write(252);
}

void getPositionData() { /* Drive subsystem coordinates are sent in the form X+127, Y+127 centimetres, this defaults to 0 at the start of execution everytime to make the system map relative to its starting point. */
  int xPos = Serial2.read() - 127; /* The UART port has an embedded FIFO (First In First Out) queue data structure, this means the initial entries are at the top of the queue, and the first elements*/
  int yPos = Serial2.read() - 127; /* So the function takes the first 2 bytes from the queue with each execution, which contain the respective X and Y positions, offset by 127. */
  float angle = ((Serial2.read() - 120) * 6.283) / 240;
  isBusy = Serial2.read();
  if (xPos == -127 && yPos == -128) { /* The ESP32 may still have been reading unflushed bytes from a previous use or prior to a reset, causing it to read serial2 bytes that are eroneous at the start, these must be filtered manually*/
    xPos = 0; /* Set these coordinates directly to 0 to manually hard filter the error. */
    yPos = 0;
  }
  if (xPos == -127 && yPos == -127) {
    xPos = 0;
    yPos = 0;
  }
  if (xPos == -127 && yPos == 128) {
    xPos = 0;
    yPos = 0;
  }
  if(xPos == -128 && yPos == -128){ /* Filters this error which comes out from autopiloting. */
    xPos = 0;
    yPos = 0;
  }
  String statusmqtt;
  if(isBusy == true) {
    statusmqtt = "true";
  }
  else if(isBusy == false) {
    statusmqtt = "false";
  }
  String xS, yS;
  xS = String(xPos).c_str();
  yS = String(yPos).c_str();
  String overall = (xS + "," + yS).c_str(); /* Converting the coordinates to two strings, and then concatonating them into the form X,Y, in order to form a single coordinate entry for each time slot. */
  Serial.println();
  Serial.print("Positional Data received via UART - X: ");
  Serial.print(xPos);
  Serial.print(", Y: ");
  Serial.print(yPos);
  Serial.print(", Oriented: ");
  Serial.print(angle);
  Serial.print(" radians from startpoint.");
  Serial.println();
  Serial.print(isBusy);
  currentcoordinates[0] = xPos;
  currentcoordinates[1] = yPos;
  currentangle = angle;
  Serial.println(); /* Printing coordinates to the main serial for debugging information. */
  client.publish("position", (char*)overall.c_str()); /* Publish this information back to Command subsystem, which is subscribed to topic 'position', ensure that the string is put into a C String format to send back. */
  client.publish("angle", (char*)((String)angle).c_str());
  client.publish("active", (char*)(statusmqtt).c_str());
}

void getVisionData() { /* Function to process Vision data. It reads the 12 bytes sent out by the FPGA and processes those, pairs of bytes correspond to distance to a certain bounding box colour, green, blue, pink, orange black, last 2 are always 0.*/
  for (int i = 0; i < 12; i++) {
    int out = Serial1.read();
    visiondistance1[i] = out; /* Serial UART port is FIFO, meaning the initial values are at the top and sequentially down, append all these to a global array.*/

    Serial.println();
    Serial.print(visiondistance1[i]);
  }
  colourarray[0] = visiondistance1[2]; /* Bytes are misaligned, but correspond to these elements of the 96 bit transmission.*/
  colourarray[1] = visiondistance1[8];
  colourarray[2] = visiondistance1[6];
  colourarray[3] = visiondistance1[4];
  colourarray[4] = visiondistance1[0];
  Serial.println();
  proximity = 0; /* Initialise primary colour and distance to primary colour flags. This effectively finds the main colour of the object, setting it 0 to 4 depending on which of the 4 colours it is, and 5 for default non colour, proximity is 0. */
  colour = 5; /* Colour set to 5 by default for NO COLOUR, will change to 0 to 4 for green, black, pink, blue, orange respectively. */
  for (int i = 0; i < 5; i++) {
    if (colourarray[i] != 0) { /* If there is a value in the updated array that is above the current proximity, and is a valid filtered colour, then set the new colour. */
      proximity = colourarray[i];
      colour = i; 
    }

  }
 
  if (colour != 5) {
    objectcoordinates[0] = currentcoordinates[0] + (proximity + 8) * cos(currentangle); /* Take proximity and offset this by distance in cm between camera and front of rover, find horizontal and vertical components and offset this to current rover positions to get rough coordinates of object*/
    objectcoordinates[1] = currentcoordinates[1] + (proximity + 8) * sin(currentangle);
  }
    if(colour == 0 && gflag == false) {
    client.publish("colour", "green");
    gflag = true;
    }
    if(colour == 1) {
     client.publish("colour", "black");
    }
    if(colour == 2 && pflag == false) {
    client.publish("colour", "pink");
    pflag = true;
    }
    if(colour == 3 && bflag == false) {
    client.publish("colour", "blue");
    bflag = true;
    }
    if(colour == 4 && oflag == false) {
    client.publish("colour", "orange");
    oflag = true;
    }
    if(colour == 5) {
    client.publish("colour", "NA");
    }
    String xO, yO;
    xO = String(objectcoordinates[0]).c_str();
    yO = String(objectcoordinates[1]).c_str();
    String overallO = (xO + "," + yO).c_str();
    client.publish("objectcoordinate", (char*)overallO.c_str());
      Serial.println();
      Serial.print("Green Distance is: ");
      Serial.print(colourarray[0]);      
      Serial.println();      
      Serial.print("Black Distance is: ");   
      Serial.print(colourarray[1]);      
      Serial.println();
      Serial.print("Pink Distance is: ");
      Serial.print(colourarray[2]);
      Serial.println();
      Serial.print("Blue Distance is: ");
      Serial.print(colourarray[3]);
      Serial.println();
      Serial.print("Orange Distance is: ");
      Serial.print(colourarray[4]);
      String proximitymqtt = String(proximity).c_str();
      Serial.println();
      Serial.print(proximity);
      client.publish("objectproximity", (char*)proximitymqtt.c_str()); /* Publish relevant proximity and colour information to the broker. */
  
    
  
  /*client.publish("vision", (char*)conv.c_str()); /* Basic function made in prep for Vision */
}

void setup() {
  int i = 0;
  Serial.begin(115200); /*This Serial port is used temporarily in this implementation for debug information. All of this information is already stored in Command, so this is an extra feature. The UART port was meant to be used for the Energy subsystem.
  However this cannot be integrated, the code can be easily adjusted to accomodate energy simply by using the function declared in above comments, which reads UART battery life information and publishes it back, and uncommenting the pin assignments for
  connection to energy system via UART, and finally deleting the Serial debug messages. Meaning that this subsystem can accomodate both with and without an Energy subsystem.*/
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); /* I am using the Serial2 for UART communication with drive, SERIAL_8N1 means I am using 8 bit bytes, where there is 1 start bit. 8 data bits per byte.*/
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1); /* This UART port is being used for Vision, this will read the colour information, and bounding box coordinates. */
  setup_wifi(); /* Connect to WIFI */
  client.setServer(mqttserver, mqttPort); /* Target the MQTT server, port. */
  client.setCallback(callback); /* Set the callback protocol to be the function declared earlier. */
}

void loop() {
  if (!client.connected()) { /* In the main loop, we will constantly check if the WIFI is connected, if not just connect to the WIFI */
    reconnect();
  }
  requestPosData(); /* Once every 250 milliseconds, request position data from the drive subsystem. */
  if (Serial2.available() > 0) { /* If the counter is above the first entry, and there are bytes available to be read in the Serial2 queue, then call the getPositionData function to get the next two coordinates.*/
    getPositionData(); 
  }
  if (Serial1.available() > 0) {
    getVisionData();
  }
//  }
  if (waypointstart == true) { 

    if (currentangle != 1.57) {
      rotateamount(180);
      unsigned long rotationstart = millis();
      unsigned long timer = 0;
      while (millis() - rotationstart < 5000) { /* millis used as alternative to delay, and allows the rover to gather position data while moving. */
        if (millis() - timer > 250) { /* Maintain consistency by sending data every 250ms */
          timer = millis();
          requestPosData();
          getPositionData();
        }

    }
      /*If the angle is different return to starting position where the thing is 1.55*/
    }
    rotateamount(int(desiredangle) + 120); /* Rotate to the globally stored desired angle subtended by the current coordinates, and the desired coordinates (offsetted by 120). */
    unsigned long anglestart = millis();
    unsigned long timer2 = 0;
    while (millis() - anglestart < 5000) {
      if (millis() - timer2 > 250) {
        timer2 = millis();
        requestPosData();
        if(Serial2.available() > 0){
        getPositionData();
        }
      }
    }
    float newamount = sqrt(sq(waypointdifferences[1]) + sq(waypointdifferences[0])) + 120; /* Fetches hypotenuse to create shortest distance */
    movedistance(int(newamount));
    unsigned long movementstart = millis();
    unsigned long timer3 = 0;
    while (millis() - movementstart < 7000) {
      if (millis() - timer3 > 250) {
        timer3 = millis();
        requestPosData();
        getPositionData(); /* Replace this with vision */
      }
    }
    Serial.println();
    Serial.print("Arrived at point.");
    waypointstart = false; /* Setting flag to false so it can be called again later. */
//    if(autopilotflag == true){
//      autopilotstart = true;
//      autopilotflag = false;
//    }
  }
  if (autopilotstart == true && waypointstart == false) {
    int xreference, yreference;
    xreference = currentcoordinates[0];
    yreference = currentcoordinates[1];
    if (autopilotcounter < autopilotcycles) {
      int xnew = random(xreference + 1, range/2);
      int ynew = random(yreference + 1, range/2);
      String autocoord = "";
      autocoord = String(xnew) + "," + String(ynew);
      autocoord = autocoord.c_str();
      Serial.println();
      Serial.print("Going to: ");
      Serial.print(autocoord);
      waypoint(autocoord);
      autopilotcounter++;
    }
    else {
      autopilotstart = false;
    }
  }

  if (proximity < 15 && colour != 5) { /* Autonomous stopping function, recognise if there is a colour detected and the distance is less than 15cm. */
    Serial.println();
    Serial.print("Oops! Obstacle detected, performing evasive manouvres now");
    Serial.println();
    Serial.print("Now bringing rover to a complete stop");
    stoprover(); /* Stops rover immediately before crashing */
    unsigned long stoppertimer = millis();
    while (millis() - stoppertimer < 2000) {
    /* Do not do anything in the time to complete the operation*/
    }
    Serial.println();
    Serial.print("Reversing away from obstacle");
    reverse(10); /* Reverse away from obstacle */
    unsigned long reversetimer = millis();
    while (millis() - reversetimer < 3000) {
  
    }
    Serial.println();
    Serial.print("Rotating away from obstacle");
    rotateamount(120);
    unsigned long rotatetimer = millis();
    while (millis() - rotatetimer < 5000) {

    }
    Serial.println();
    Serial.print("Obstacle averted...you're welcome.");
  }
//  if (proximity < 15 && colour != 5) {
//    if (autopilotstart == true) {
//      autopilotstart = false;
//      autopilotflag = true;
//    }
//    waypointstart == true;
//    stoprover();
//    int a, b;
//    a = currentcoordinates[0] - 20 * cos(currentangle);
//    b = currentcoordinates[1] - 20 * sin(currentangle);
//    String reversecoord = "";
//    reversecoord = String(a) + "," + String(b);
//    waypoint2(reversecoord);
//  }
  client.loop(); /* Loop continuously */
  delay(250);
}
