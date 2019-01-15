/*
  A basic lightning detector Arduino example sketch. 
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
int noiseFloor = 2;

// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector. 
int intVal = 0;

void setup()
{
  // When lightning is detected the interrupt pin goes HIGH.
  pinMode(lightningInt, INPUT); 

  Serial.begin(115200); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

  //SPI.begin() 
  Wire.begin(); // Begin Wire before lightning sensor. 

  if( !lightning.begin() ) { // Initialize the sensor. 
  //if( !lightning.beginSPI(9, 2000000){ // Uncomment for SPI.
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
  if(digitalRead(lightningInt) == HIGH){
    // Hardware has alerted us to an event, now we read the interrupt register
    // to see exactly what it is. 
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
