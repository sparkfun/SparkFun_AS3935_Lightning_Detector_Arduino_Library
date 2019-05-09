/*
  --A basic lightning detector Arduino example sketch--
  Utilizing SPI, this example listens for lightning "events". The IC determines
  whether or not it's actual lightning, disturber, or noise. In the case
  your environment has a lot of noise 
  By: Elias Santistevan
  SparkFun Electronics
  Date: May, 2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Hardware: 
  This is SparkFun's Qwiic Lightning Detector and so is compatible with the Qwiic
  system. You can attach a Qwiic cable or solder to the I-squared-C pins.
  You'll also need a wire attached to the interrupt.  
*/

#include <SPI.h>
#include <Wire.h>
#include "SparkFun_AS3935.h"

#define INDOOR 0x12 
#define OUTDOOR 0xE
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04
#define NOISE_INT 0x01

SparkFun_AS3935 lightning;

// Interrupt pin for lightning detection 
const int lightningInt = 4; 
int noiseFloor = 2;
int spiCS = 10; //SPI chip select pin

// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector. 
int intVal = 0;

void setup()
{
  // When lightning is detected the interrupt pin goes HIGH.
  pinMode(lightningInt, INPUT); 

  Serial.begin(115200); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

  SPI.begin(); 

  if( !lightning.beginSPI(spiCS, 2000000) ){ 
    Serial.println ("Lightning Detector did not start up, freezing!"); 
    while(1); 
  }
  else
    Serial.println("Schmow-ZoW, Lightning Detector Ready!");

  // The lightning detector defaults to an indoor setting at 
  // the cost of less sensitivity, if you plan on using this outdoors 
  // uncomment the following line:
  //lightning.setIndoorOutdoor(OUTDOOR); 
}

void loop()
{
   // Hardware has alerted us to an event, now we read the interrupt register
  if(digitalRead(lightningInt) == HIGH){
    intVal = lightning.readInterruptReg();
    if(intVal == NOISE_INT){
      Serial.println("Noise."); 
      //reduceNoise(); //See note below above reduceNoise function.
    }
    else if(intVal == DISTURBER_INT){
      Serial.println("Disturber."); 
    }
    else if(intVal == LIGHTNING_INT){
      Serial.println("Lightning Strike Detected!"); 
      // Lightning! Now how far away is it? Distance estimation takes into
      // account any previously seen events in the last 15 seconds. 
      byte distance = lightning.distanceToStorm(); 
      Serial.print("Approximately: "); 
      Serial.print(distance); 
      Serial.println("km away!"); 
    }
  }
  delay(100); //Let's not be too crazy.
}

// This function helps to adjust the sensor to your environment. More
// environmental noise leads to more false positives. If you see lots of noise
// events, try increasing the noise threshold with this function. I put the
// function call under the if statement checking for noise. The datsheet
// warns that smartphone and smart watch displays, DC-DC converters, and/or
// anything that operates in 500 kHz range are noise sources to be avoided. 
void reduceNoise(){
  ++noiseFloor; // Manufacturer's default is 2 with a max of 7. 
  if(noiseFloor > 7){
    Serial.println("Noise floor is at max!"); 
    return;
  }
  Serial.println("Increasing the event threshold.");
  lightning.setNoiseLevel(noiseFloor);  
}

// This function is similar to the one above in that it will increase the
// antenna's robustness against false positives. However, this function helps to
// increase the robustness against "distrubers" and not "noise". If you have a
// lot of disturbers trying increasing the threshold. The default value is 2 and
// goes up to 10. You can remove the call to watchdogThreshold() from this
// function; the function is a demonstrative tool.  
void changeDisturberThresh(int threshVal){
    lightning.watchdogThreshold(threshVal);  
}
