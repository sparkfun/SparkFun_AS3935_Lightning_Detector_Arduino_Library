/*
  This example sketch will walk you through how to tune the resonance frequency
  of the antenna. Testing with a Saleae Logic 8 Pro, I've found that freshly
  manufactured boards here at SparkFun will have a frequency around ~512Hz (2.4
  percent deviation) by default. This falls well within the optimal value of 3.5 percent
  suggested in the datasheet on page 35. However if you'd like to tune that
  frequency, the chip provides internal capacitance that can be modified by the
  library. You'll need a logic analyzer, oscillscope, or some method of reading 
  a 32kHz square wave (more on that below by division ratio function call) connected to the "INT" pin.
  By: Elias Santistevan
  SparkFun Electronics
  Date: April, 2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
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
#define ANTFREQ 3

// If you're using I-squared-C then keep the following line. Address is set to
// default. 
SparkFun_AS3935 lightning(AS3935_ADDR);
// If using SPI, uncomment the line below. 
//SparkFun_AS3935 lightning;

void setup()
{
  Serial.begin(115200); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

  // SPI.begin() // For SPI
  Wire.begin(); // Begin Wire before lightning sensor. 
  if( !lightning.begin() ){ // Initialize the sensor. 
  //if( !lightning.beginSPI(9, 2000000) ) { //Uncomment for SPI
    Serial.println ("Lightning Detector did not start up, freezing!"); 
    while(1); 
  }
  else
    Serial.println("Lightning Detector ready to tune antenna!");

  // When reading the frequency, keep in mind that the given frequency is
  // divided by 16 by default. This can be changed to be divided by 32, 64, or
  // 128 using the line below. So for example when reading the frequency on a 
  // fresh board, the frequency I'm reading is 32.05kHz; multiplied by 16 and
  // the result is 512.8kHz. This is 2.56 percent away from the 500kHz ideal
  // value and well within the 3.5 percent optimal tolerance. If I were to
  // change the division ratio to 32, then I read a value of 16kHz.  
  //lightning.changeDivRatio(32);

  // The following function call is just a sanity check. It will return a value
  // of 16 by default but can be 32, 64, or 128, depending on what you change
  // it too.
  byte regVal = lightning.readDivisionRatio(); 
  Serial.println(regVal); 

  // This will tell the IC to display the resonance frequncy as a digital
  // signal on the interrupt pin. There are two other internal oscillators 
  // within the chip that can also be displayed on this line but is outside the
  // scope of this example, see page 35 of the datsheet for more information.
  lightning.displayOscillator(true, ANTFREQ); 
  // To stop displaying the frequncy on the interrupt line, give "false" as a
  // parameter.
  //lightning.displaying(false, ANTFREQ); 
  
  //Here you give a value of 0-16, which increases the capacitance on
  //the RLC circuit in steps of 8pF, up to 120pF. 
  //The change in frequency is very modest. At 15 (max), I found that the
  //frequency was sitting around 490kHz down from 512kHz. Consider that 
  //the equation for calculating frequency in an RLC circuit is: 
  // f = 1/(2pi*sqrt(LC)), just a mental morsel. 
  lightning.tuneCap(1); 
}

void loop() {
}


