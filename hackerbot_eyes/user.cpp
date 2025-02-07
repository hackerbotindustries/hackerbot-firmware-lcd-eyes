#if 1

#include <SerialCmd.h>
#include "globals.h"
#include "HackerbotSerialCmd.h"

/*
Credit in these posts
https://github.com/Lupo-Grigio/DragonCostume/tree/main/Dragon%20Head/Eyes/M4_Eyes
https://twitter.com/NoirTalon/status/1478232192761282562?s=20
https://forums.adafruit.com/viewtopic.php?p=904534
*/

// Make Sure to #define SERIAL_CONTROL in globals.h or as a build option

// This file is based on the user.cpp template
// following the form of the previously approved user_* additions
// the idea is this file gets renamed to user.cpp

// Wiring Instructions
// To be VERY CLEAR
// When wiring up the monster mask to use this code
// * look at the monster mask from the front where the tft screens are
// * look at the MEMS PWM Microphone JST connector, it has 4 tiny little pins
// * The above code makes the far left pin a serial recieve pin RX (usually the yellow wire on the JST wires)
// * The second pin from the left becomes serial TX
// * The other 2 pins are +5 and Ground. You can _almost_ ignore those unless you want to power whatever is sending you serial (not recommended for anything that takes more current than a PWM microphone)
// * it is good pratice to tie the ground to ground somewhere, and for long cables twist the cable bundle gently so the ground wire wraps around TX and RX

#define SERIAL1_BAUD 115200 // make sure this matches the rate at which your sender is transmitting

// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global.
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop
String IncomingString = String("");


// HERE BE MAGIC!
// This next block of code sets up the second UART on the SAMD51 to act as a serial port
// On the Monster M4SK this turns the PWM Micraphone port into a serial connection
// If you google this code you will find it in a couple places with vague descriptions
// of what it does. All of these definitions are buried deep in the Adafruit libraries
// I wish I could document all these definitions... If you import this project into platform.io
// running in VSCode you can actually use VSCode to referse-figureout what all this does
// in the Arduino IDE... lots of luck....
Uart Serial1( &sercom3, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX ) ;
void SERCOM3_0_Handler() { Serial1.IrqHandler(); }
void SERCOM3_1_Handler() { Serial1.IrqHandler(); }
void SERCOM3_2_Handler() { Serial1.IrqHandler(); }
void SERCOM3_3_Handler() { Serial1.IrqHandler(); }

// Set up the serial command processor
HackerbotSerialCmd mySerCmd(Serial1);

static int count = 0;

// HOW TO WIRE!
// To be VERY CLEAR
// When wiring up the monster mask to use this code
// * look at the monster mask from the front where the tft screens are
// * look at the MEMS PWM Microphone JST connector, it has 4 tiny little pins
// * The above code makes the far left pin a serial recieve pin RX (usually the yellow wire on the JST wires)
// * The second pin from the left becomes serial TX
// * The other 2 pins are +5 and Ground. You can _almost_ ignore those unless you want to power whatever is sending you serial (not recommended for anything that takes more current than a PWM microphone)
// * it is good pratice to tie the ground to ground somewhere, and for long cables twist the cable bundle gently so the ground wire wraps around TX and RX

void sendOK(void) {
  mySerCmd.Print((char *) "OK\r\n");
}

void set_GAZE(void) {
  float newEyeTargetX = 0.0;
  float newEyeTargetY = 0.0;

  if (!mySerCmd.ReadNextFloat(&newEyeTargetX) || !mySerCmd.ReadNextFloat(&newEyeTargetY)) {
    mySerCmd.Print((char *) "ERROR: Missing parameter\r\n");
    return;
  }

  moveEyesRandomly = false;

  // Constrain values to acceptable range
  eyeTargetX = constrain(newEyeTargetX, -1.0, 1.0);
  eyeTargetY = constrain(newEyeTargetY, -1.0, 1.0);

  char buf[128] = {0};
  sprintf(buf, "STATUS: Updated: eyeTarget: %0.2f, %0.2f\r\n", eyeTargetX, eyeTargetY);
  mySerCmd.Print(buf);

  count = 0;

  sendOK();
}

void user_setup(void) 
{
    //if(!Serial)
    //{
      //Serial.begin(DEBUG_BAUD);
/*      while (!Serial)
        yield();*/
    //}
  char buf[128] = {0};

  mySerCmd.Print((char *) "\r\n\r\n");
  mySerCmd.Print((char *) "Serial1 control over Monster M4SK Eyes\r\n");
  sprintf(buf, "Setting up PWM port to be Serial port 1 TX and RX are %d %d\r\n",PIN_SERIAL1_TX,PIN_SERIAL1_RX);
  mySerCmd.Print(buf);

  Serial1.begin(SERIAL1_BAUD);
  
  while (!Serial1)
  {
    mySerCmd.Print((char *) ".");
  }
  sprintf(buf, "Serial1 started successfully at %d Baud and ready to read\r\n",SERIAL1_BAUD);
  mySerCmd.Print(buf);

  mySerCmd.AddCmd("GAZE", SERIALCMD_FROMALL, set_GAZE);
}

// Called once after the processing of the configuration file. This allows
// user configuration to also be done based on the config file.
#include <ArduinoJson.h>          // JSON config file functions
void user_setup(StaticJsonDocument<2048> &doc) {
}

void user_loop(void) {
  int8_t ret;

  // Check for and run incoming serial commands
  ret = mySerCmd.ReadSer();
  if (ret == 1) { // we processed a command, reset the return-to-random counter:
    count = 0;
  } else {
    if (ret == 0) {
      mySerCmd.Print((char *) "ERROR: Urecognized command\r\n");
    }
    count++;
    if (count > 100) { // After no valid override command for 100 ticks, return to random movement
      moveEyesRandomly = true;
    }
  }
}


#endif // 0 && !defined(SERIAL_CONTROL) 
