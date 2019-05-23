/*
  A more in depth lightning detector Arduino example sketch. 
  This will guide you through some more of the library's functions that aid in
  reducing false events and noise. Most of the functions are also coupled by  
  By: Elias Santistevan
  SparkFun Electronics
  Date: January, 2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  This example listens for lightning events, which are internally determined by
  the IC to be real or false events. 
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

// SPI
SparkFun_AS3935 lightning;


// Interrupt pin for lightning detection 
const int lightningInt = 4; 
// Chip select pin 
int spiCS = 10; 

// Values for modifying the IC's settings. All of these values are set to their
// default values. 
byte noiseFloor = 2;
byte watchDogVal = 2;
byte spike = 2;
byte lightningThresh = 1; 


// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector. 
byte intVal = 0; 

void setup()
{
  // When lightning is detected the interrupt pin goes HIGH.
  pinMode(lightningInt, INPUT); 

  Serial.begin(115200); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

   SPI.begin(); // For SPI
  if( !lightning.beginSPI(spiCS, 2000000) ) { 
    Serial.println ("Lightning Detector did not start up, freezing!"); 
    while(1); 
  }
  else
    Serial.println("Schmow-ZoW, Lightning Detector Ready!\n");

  // "Disturbers" are events that are false lightning events. If you find
  // yourself seeing a lot of disturbers you can have the chip not report those
  // events on the interrupt lines. 
  lightning.maskDisturber(true); 
  int maskVal = lightning.readMaskDisturber();
  Serial.print("Are disturbers being masked: "); 
  if (maskVal == 1)
    Serial.println("YES"); 
  else if (maskVal == 0)
    Serial.println("NO"); 

  // The lightning detector defaults to an indoor setting (less
  // gain/sensitivity), if you plan on using this outdoors 
  // uncomment the following line:
  lightning.setIndoorOutdoor(OUTDOOR); 
  int enviVal = lightning.readIndoorOutdoor();
  Serial.print("Are we set for indoor or outdoor: ");  
  if( enviVal == INDOOR )
    Serial.println("Indoor.");  
  else if( enviVal == OUTDOOR )
    Serial.println("Outdoor.");  
  else 
    Serial.println(enviVal, BIN); 

  // Noise floor setting from 1-7, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.    
  //lightning.setNoiseLevel(noiseFloor);  
  int noiseVal = lightning.readNoiseLevel();
  Serial.print("Noise Level is set at: ");
  Serial.println(noiseVal);

  // Watchdog threshold setting can be from 1-10, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.    
  //lightning.watchdogThreshold(watchDogVal); 
  int watchVal = lightning.readWatchdogThreshold();
  Serial.print("Watchdog Threshold is set to: ");
  Serial.println(watchVal);

  // Spike Rejection setting from 1-11, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.    
  // The shape of the spike is analyzed during the chip's
  // validation routine. You can round this spike at the cost of sensitivity to
  // distant events. 
  //lightning.spikeRejection(spike); 
  int spikeVal = lightning.readSpikeRejection();
  Serial.print("Spike Rejection is set to: ");
  Serial.println(spikeVal);


  // This setting will change when the lightning detector issues an interrupt.
  // For example you will only get an interrupt after five lightning strikes
  // instead of one. Default is one, and it takes settings of 1, 5, 9 and 16.   
  // Followed by its corresponding read function. Default is zero. 
  //lightning.lightningThreshold(lightningThresh); 
  uint8_t lightVal = lightning.readLightningThreshold();
  Serial.print("The number of strikes before interrupt is triggerd: "); 
  Serial.println(lightVal); 

  // When the distance to the storm is estimated, it takes into account other
  // lightning that was sensed in the past 15 minutes. If you want to reset
  // time, then you can call this function. 
  //lightning.clearStatistics(); 

  // This will power down your lightning detector. You absolutely have to call
  // the wakup funciton following, because it recalibrates the internal
  // oscillators. 
  //lightning.powerDown(); 
  //if( lightning.wakeUp() ) 
  //  Serial.println("Successfully woken up!");  
  //else 
  //  Serial.println("Error recalibrating internal osciallator on wake up."); 

}

void loop()
{
  if(digitalRead(lightningInt) == HIGH){
    // Hardware has alerted us to an event, now we read the interrupt register
    // to see exactly what it is. 
    intVal = lightning.readInterruptReg();
    if(intVal == NOISE_INT){
      Serial.println("Noise."); 
    }
    else if(intVal == DISTURBER_INT){
      Serial.println("Disturber."); 
    }
    else if(intVal == LIGHTNING_INT){
      Serial.println("Lightning Strike Detected!"); 
      // Lightning! Now how far away is it? Distance estimation takes into
      // account previously seen events. 
      byte distance = lightning.distanceToStorm(); 
      Serial.print("Approximately: "); 
      Serial.print(distance); 
      Serial.print("km away!"); 
      // "Lightning Energy" and I do place into quotes intentionally, is a pure
      // number that does not have any physical meaning. 
      //long lightEnergy = lightning.lightningEnergy(); 
      //Serial.print("Lightning Energy: "); 
      //Serial.println(lightEnergy); 

    }
  }
}

