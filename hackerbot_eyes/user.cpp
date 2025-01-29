#if 1

#include "globals.h"

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

bool HandleSerialInput();

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

// HOW TO WIRE!
// To be VERY CLEAR
// When wiring up the monster mask to use this code
// * look at the monster mask from the front where the tft screens are
// * look at the MEMS PWM Microphone JST connector, it has 4 tiny little pins
// * The above code makes the far left pin a serial recieve pin RX (usually the yellow wire on the JST wires)
// * The second pin from the left becomes serial TX
// * The other 2 pins are +5 and Ground. You can _almost_ ignore those unless you want to power whatever is sending you serial (not recommended for anything that takes more current than a PWM microphone)
// * it is good pratice to tie the ground to ground somewhere, and for long cables twist the cable bundle gently so the ground wire wraps around TX and RX


void user_setup(void) 
{
    //if(!Serial)
    //{
      //Serial.begin(DEBUG_BAUD);
/*      while (!Serial)
        yield();*/
    //}
  Serial.println();
  Serial.println();
  Serial.println("Serial1 control over Monster M4SK Eyes");
  Serial.printf("Setting up PWM port to be Serial port 1 TX and RX are %d %d",PIN_SERIAL1_TX,PIN_SERIAL1_RX);
  Serial.println();

  Serial1.begin(SERIAL1_BAUD);
  
  while (!Serial1)
  {
    Serial.print(".");
  }
  Serial.printf("Serial1 started successfully at %d Baud and ready to read",SERIAL1_BAUD);
  Serial.println();
}

// Called once after the processing of the configuration file. This allows
// user configuration to also be done based on the config file.
#include <ArduinoJson.h>          // JSON config file functions
void user_setup(StaticJsonDocument<2048> &doc) {
}

// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop

bool HandleSerialInput()
{
  char c;
  bool ret = false;
  if(Serial1.available()>0) // hey there is something there
  {
    while(Serial1.available()>0)
    {
          IncomingString += (char)Serial1.read();
    }
    
    if(IncomingString.endsWith("\n") || IncomingString.endsWith("\n "))
    {
      //Serial.printf("Received: '%s'", IncomingString.c_str());
      //Serial.println();

      int8_t newEyeTargetX, newEyeTargetY = 0;
      IncomingString.trim();
      //Serial.printf("Parsing '%s'...", IncomingString.c_str());
      //Serial.println();
      sscanf(IncomingString.c_str(), "GAZE,%d,%d", &newEyeTargetX, &newEyeTargetY);

      //Serial.printf("Updating gaze - newEyeTarget: %d, %d", newEyeTargetX, newEyeTargetY);
      //Serial.println();

      moveEyesRandomly = false; // stop random eye movement TODO: Time this to prevent jerking

      eyeTargetX = (float(newEyeTargetX) / 100.0);
      eyeTargetY = (float(newEyeTargetY) / 100.0);

      Serial.printf("Updated eyeTarget: %.2f, %.2f", eyeTargetX, eyeTargetY);
      Serial.println();
      Serial.println();

      ret = true;
      IncomingString.remove(0); // empty the string
    }
    else
    {
      // incomplete message, just throw it away, something may be going wrong
      Serial.printf("Reached end of buffer before EOM: '%s'", IncomingString.c_str());
      Serial.println();
    }
  }
  return(ret); 

}

static int count = 0;
void user_loop(void) {


  if (!HandleSerialInput())
  {
    count++;
    if(count > 100) // no faces found so we do a non time based delay before randomly looking around again
     moveEyesRandomly = true;
  }
  else
  {
    count = 0;
  }

}


#endif // 0 && !defined(SERIAL_CONTROL) 
