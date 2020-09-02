/*
  This example sketch will walk you through how to tune the resonance frequency
  of the antenna using the IC's internal tuning caps. You'll need a logic analyzer, 
  oscillscope, or some method of reading a square wave of at least 4kHz but up to 32kHz.
  A note on what you can expect from a board fresh from SparkFun.
  The resonance frequency of a freshly manufactured SparkFun AS3935 Lightning
  Detectorw has been ~496kHz which is less then one percent deviation from
  perfect resonance. This falls well within the optimal value of 3.5 percent
  suggested in the datasheet on page 35. Again, 3.5 percent is OPTIMAL so try
  not to tear your hair out if it's not perfect. 

  By: Elias Santistevan
  SparkFun Electronics
  Date: April, 2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

*/

#include <SPI.h>
#include <Wire.h>
#include "SparkFun_AS3935.h"

#define ANTFREQ 3

// Chip select for SPI on pin ten.
int spiCS = 10; 

// SPI
SparkFun_AS3935 lightning;

void setup()
{
  Serial.begin(115200); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

  SPI.begin(); // For SPI
  if( !lightning.beginSPI(spiCS) ) { 
    Serial.println ("Lightning Detector did not start up, freezing!"); 
    while(1); 
  }
  else
    Serial.println("Ready to tune antenna!");

  // The frequency of the antenna is divided by the "division ratio" which is
  // set to 16 by default. This can be changed to 32, 64, or 128 using the 
  // function call below. As an example when reading the frequency of a 
  // new board, the frequency 31.04kHz. Multiplying that frequency by the
  // division ratio gives 496.64kHz, which is less than 1 percent within the
  // OPTIMAL range of 3.5 percent specified in the datasheet.

  //lightning.changeDivRatio(32);

  // Read the division ratio - 16 is default.  
  byte divVal = lightning.readDivRatio(); 
  Serial.print("Division Ratio is set to: "); 
  Serial.println(divVal); 

  // Here you give the value of the capacitor you want turned on. It accepts up
  // to 120pF in steps of 8pF: 8, 16, 24, 32 etc.The change in frequency is
  // somewhat modest. At the maximum value you can lower the frequency up to 22kHz. 
  // As a starting point, the products designed in house ship around 496kHz
  // (though of course every board is different) putting you within one percent
  // of a perfect resonance; the datasheet specifies being within 3.5 percent as
  // optimal. 

  //lightning.tuneCap(8); 

  // When reading the internal capcitor value, it will return the value in pF.
  int tuneVal = lightning.readTuneCap();
  Serial.print("Internal Capacitor is set to: "); 
  Serial.println(tuneVal);

  // This will tell the IC to display the resonance frequncy as a digital
  // signal on the interrupt pin. There are two other internal oscillators 
  // within the chip that can also be displayed on this line but is outside the
  // scope of this example, see page 35 of the datsheet for more information.
  Serial.println("\n----Displaying oscillator on INT pin.----\n"); 
  lightning.displayOscillator(true, ANTFREQ); 

  // To stop displaying the frequncy on the interrupt line, give "false" as a
  // parameter or power down your lightning detector.

  //lightning.displayOscillator(false, ANTFREQ); 

  // You can now calibrate the internal oscillators of the IC - given that the
  // resonance frequency of the antenna is tightly trimmed. They are calibrated
  // off of the antenna frequency. 

  //if(lightning.calibrateOsc());
      //Serial.println("Successfully Calibrated!");
    //else
      //Serial.println("Not Successfully Calibrated!");
}

void loop() {
}


