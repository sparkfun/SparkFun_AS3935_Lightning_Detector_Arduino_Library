/*
  A more in depth lightning detector Arduino example sketch. 
  This will guide you through some more of the library's functions that aid in
  reducing false events and noise. 
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

// 0x03 is default, but the address can also be 0x02, 0x01, or 0x00
// Adjust the address jumpers on the underside of the product. 
#define AS3935_ADDR 0x03 
#define INDOOR 0x12 
#define OUTDOOR 0xE
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04
#define NOISE_INT 0x01

// If you using SPI, instantiate class without address: 
//SparkFun_AS3935 lightning;

// If you're using I-squared-C then keep the following line. Address is set to
// default. 
SparkFun_AS3935 lightning(AS3935_ADDR);

// Interrupt pin for lightning detection 
const int lightningInt = 4; 
// Values for modifying the IC's detection sensitivity. 
byte noiseFloor = 2;
byte watchDogVal = 2;
byte spike = 2;

// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector. 
byte intVal = 0; 

void setup()
{
  // When lightning is detected the interrupt pin goes HIGH.
  pinMode(lightningInt, INPUT); 

  Serial.begin(115200); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

  // SPI.begin(); // For SPI
  Wire.begin(); // Begin Wire before lightning sensor. 
  if( !lightning.begin() ){ // Initialize the sensor. 
  //if( !lightning.beginSPI(9, 2000000) ) { //Uncomment for SPI
    Serial.println ("Lightning Detector did not start up, freezing!"); 
    while(1); 
  }
  else
    Serial.println("Schmow-ZoW, Lightning Detector Ready!");
  // "Disturbers" are events that are false lightning events. If you find
  // yourself seeing a lot of disturbers you can have the chip not report those
  // events on the interrupt lines. 
  //lightning.maskDisturber(true); 

  // The lightning detector defaults to an indoor setting (less
  // gain/sensitivity), if you plan on using this outdoors 
  // uncomment the following line:
  //lightning.setIndoorOutdoor(OUTDOOR); 
  
}

void loop()
{
  if(digitalRead(lightningInt) == HIGH){
    // Hardware has alerted us to an event, now we read the interrupt register
    // to see exactly what it is. 
    intVal = lightning.readInterruptReg();
    if(intVal == NOISE_INT){
      Serial.println("Noise."); 
      // reduceNoise();  See note below above function definition.
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

// This function helps to adjust the sensor to your environment. More
// environmental noise leads to more false positives. If you see lots of noise
// events, try increasing the noise threshold with this function. I put the
// function call under the if statement checking for noise. The datsheet
// warns that smartphone and smart watch displays, DC-DC converters, and/or
// anything that operates in 500 kHz range are noise sources to be avoided. 
void reduceNoise()
{
  ++noiseFloor; // Manufacturer's default is 2 with a max of 7. 
  if(noiseFloor > 7){
    Serial.println("Noise floor is at max!"); 
    return;
  }
  Serial.println("Increasing the event threshold.");
  lightning.setNoiseLevel(noiseFloor);  
}

// This function increases the threshold for events that are displayed on the
// interrupt pin. Increase value means more powerful events will need to occur
// to be cause the interrupt pin to go high. 
void increaseWatchdog()
{
  ++watchDogVal; // Starting at defult for demonstrative purposes and so we'll need to increase value before we write it. 
  if(watchDogVal > 7){
    Serial.println("Watchdog threshold is already at max."); 
    return; 
  }
  lightning.watchdogThreshold(watchDogVal); 
}

// This function like the watchdog above can help with to tune your detector to
// more accurate readings. The shape of the pike is analyzed during the chip's
// validation routine. You can round this spike at the cost of sensitivity to
// distant events. 
void dullTheSpike()
{
  ++spike; 
  if(spike > 11) { 
    Serial.println("Spike reduction is at mask!"); 
    return; 
  }
  lightning.spikeRejection(spike); 
}

// The following two functions should be called as a pair. If you're powering
// the IC down, then you should probably wake it up ;). The IC will consume
// 1-2uA while powered down. 
void powerICDown()
{
  Serial.println("Powering down the IC for deep sleep."); 
  lightning.powerDown(); 
}

void wakeItUP()
{
  Serial.println("Waking it up!"); 
  if( lightning.wakeUp() ) 
    Serial.println("Successfully woken up!");  
  else 
    Serial.println("Error recalibrating internal osciallator on wake up."); 
}
