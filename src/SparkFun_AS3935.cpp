/*
  This is a library for the ASM AS3935 Franklin Lightning Detector
  By: Elias Santistevan
  SparkFun Electronics
  Date: January, 2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
*/


#include "SparkFun_AS3935.h"

// Default constructor, to be used with SPI
SparkFun_AS3935::SparkFun_AS3935() { }

// Another constructor with I2C but receives address from user.  
SparkFun_AS3935::SparkFun_AS3935(i2cAddress address) { int(_address) = address; }

bool SparkFun_AS3935::begin( TwoWire &wirePort )
{
  // Startup time requires 2ms for the LCO and 2ms more for the RC oscillators
  // which occurs only after the LCO settles. See "Timing" under "Electrical
  // Characteristics" in the datasheet.  
  delay(4); 
  _i2cPort = &wirePort;
  //  _i2cPort->begin(); A call to Wire.begin should occur in sketch 
  //  to avoid multiple begins with other sketches.

  // A return of 0 indicates success, else an error occurred. 
  _i2cPort->beginTransmission(_address);
  uint8_t _ret = _i2cPort->endTransmission();
  if(!_ret)
    return true;
  else
    return false; 
}

bool SparkFun_AS3935::beginSPI(uint8_t user_CSPin, uint32_t spiPortSpeed, SPIClass &spiPort) 
{
  // Startup time requires 2ms for the LCO and 2ms more for the RC oscillators
  // which occurs only after the LCO settles. See "Timing" under "Electrical
  // Characteristics" in the datasheet.  
  delay(4);
  // I'll be using this as my indicator that SPI is to be used and not I2C.   
  _i2cPort = NULL; 
  _spiPort = &spiPort; 
  _spiPortSpeed = spiPortSpeed; // Make sure it's not 500kHz or it will cause feedback with antenna.
  _cs = user_CSPin;
  pinMode(_cs, OUTPUT); 
  digitalWrite(_cs, HIGH);// Deselect the Lightning Detector. 
  _spiPort->begin(); // Set up the SPI pins. 

  // Bit order is different for ESP32
#ifdef ESP32 
    SPI.setBitOrder(SPI_MSBFIRST);
#else
    SPI.setBitOrder(MSBFIRST);
#endif

  return true; 
}
// REG0x00, bit[0], manufacturer default: 0. 
// The product consumes 1-2uA while powered down. If the board is powered down 
// the the TRCO will need to be recalibrated: REG0x08[5] = 1, wait 2 ms, REG0x08[5] = 0.
// SPI and I-squared-C remain active when the chip is powered down. 
void SparkFun_AS3935::powerDown()
{
  writeRegister(AFE_GAIN, 0x1, 1, 0); 
}

// REG0x3A bit[7].
// This register holds the state of the timer RC oscillator (TRCO),
// after it has been calibrated. The TRCO in this case needs to be recalibrated
// after power down. The following function wakes the IC, sends the "Direct Command" to 
// CALIB_RCO register REG0x3D, waits 2ms and then checks that it has been successfully
// calibrated. Note that I-squared-C and SPI are active during power down. 
bool SparkFun_AS3935::wakeUp()
{
  writeRegister(AFE_GAIN, 0x1, 0, 0); // Set the power down bit to zero to wake it up
  writeRegister(CALIB_RCO, 0x0, DIRECT_COMMAND, 0); // Send command to calibrate the oscillators 
  delay(2); // Give time for the internal oscillators to start up.  
  if( readRegister(CALIB_SRCO, 1) && CALIB_MASK ) 
    return true;
  else
    return false; 
}

// REG0x00, bits [5:1], manufacturer default: 10010 (INDOOR). 
// This funciton changes toggles the chip's settings for Indoors and Outdoors. 
void SparkFun_AS3935::setIndoorOutdoor( uint8_t _setting )
{
  if((_setting == INDOOR) || (_setting == OUTDOOR))
    { }
  else
    return;

  if(_setting == INDOOR)
    writeRegister(AFE_GAIN, GAIN_MASK, INDOOR, 1); 
  if(_setting == OUTDOOR)
    writeRegister(AFE_GAIN, GAIN_MASK, OUTDOOR, 1); 
}

// REG0x01, bits[3:0], manufacturer default: 0010 (2). 
// This setting determines the threshold for events that trigger the 
// IRQ Pin.  
void SparkFun_AS3935::watchdogThreshold( uint8_t _sensitivity )
{
  if( (_sensitivity < 1) || (_sensitivity > 10) )// 10 is the max sensitivity setting
    return; 

  writeRegister(THRESHOLD, GAIN_MASK, _sensitivity, 1); 
}

// REG0x01, bits [6:4], manufacturer default: 010 (2).
// The noise floor level is compared to a known reference voltage. If this
// level is exceeded the chip will issue an interrupt to the IRQ pin,
// broadcasting that it can not operate properly due to noise (INT_NH).
// Check datasheet for specific noise level tolerances when setting this register. 
void SparkFun_AS3935::setNoiseLevel( uint8_t _floor )
{
  if( (_floor < 1) || (_floor > 7) )
    return; 
  
  writeRegister(THRESHOLD, FLOOR_MASK, _floor, 4); 
}

// REG0x02, bits [3:0], manufacturer default: 0010 (2).
// This setting, like the watchdog threshold, can help determine between false
// events and actual lightning. The shape of the spike is analyzed during the
// chip's signal validation routine. Increasing this value increases robustness
// at the cost of sensitivity to distant events. 
void SparkFun_AS3935::spikeRejection( uint8_t _spSensitivity )
{
  if( (_spSensitivity < 1) || (_spSensitivity > 11) )
    return; 

  writeRegister(LIGHTNING_REG, SPIKE_MASK, _spSensitivity, 0); 
}


// REG0x02, bits [5:4], manufacturer default: 0 (single lightning strike).
// The number of lightning events before IRQ is set high. 15 minutes is The 
// window of time before the number of detected lightning events is reset. 
// The number of lightning strikes can be set to 1,5,9, or 16. 
void SparkFun_AS3935::lightningThreshold( uint8_t _strikes )
{

  if( (_strikes == 1) || (_strikes == 5) || (_strikes == 9) || (_strikes == 16) )
    { } 
  else
    return; 

  if( _strikes == 1)
    writeRegister(LIGHTNING_REG, ((1<<5)|(1<<4)), 0, 4); //Demonstrative
  if( _strikes == 5)
    writeRegister(LIGHTNING_REG, ((1<<5)|(1<<4)), 1, 4); 
  if( _strikes == 9)
    writeRegister(LIGHTNING_REG, ((1<<5)|(1<<4)), 1, 5); 
  if( _strikes == 16)
    writeRegister(LIGHTNING_REG, ((1<<5)|(1<<4)), (1<<5)|(1<<4), 4); 
}


// REG0x02, bit [6], manufacturer default: 1. 
// This register clears the number of lightning strikes that has been read in
// the last 15 minute block. 
void SparkFun_AS3935::clearStatistics(bool _clearStat)
{
  if(_clearStat != true)
    return;
  //Write high, then low, then high to clear.
  writeRegister(LIGHTNING_REG, (1<<6), 1, 6);
  writeRegister(LIGHTNING_REG, (1<<6), 0, 6); //Demonstrative
  writeRegister(LIGHTNING_REG, (1<<6), 1, 6);
}

// REG0x03, bits [3:0], manufacturer default: 0. 
// When there is an event that exceeds the watchdog threshold, the register is written
// with the type of event. This consists of two messages: INT_D (disturber detected) and 
// INT_L (Lightning detected). A third interrupt INT_NH (noise level too HIGH) 
// indicates that the noise level has been exceeded and will persist until the
// noise has ended. Events are active HIGH. There is a one second window of time to
// read the interrupt register after lightning is detected, and 1.5 after
// disturber.  
uint8_t SparkFun_AS3935::readInterruptReg()
{
    // A 2ms delay is added to allow for the memory register to be populated 
    // after the interrupt pin goes HIGH. See "Interrupt Management" in
    // datasheet. 
    delay(2);
    uint8_t _interValue; 
    _interValue = readRegister(INT_MASK_ANT, 1); 
    _interValue &= (~INT_MASK); // Only need the first four bits [3:0]
    return(_interValue); 
}

// REG0x03, bit [5], manufacturere default: 0.
// This setting will change whether or not disturbers trigger the IRQ Pin. 
void SparkFun_AS3935::maskDisturber(bool _state)
{
  if(_state == true)
    writeRegister(INT_MASK_ANT, (1<<5), 1, 5); 
  if(_state == false)
    writeRegister(INT_MASK_ANT, (1<<5), 0, 5); //Demonstrative
  
}

// REG0x03, bit [7:6], manufacturer default: 0 (16 division ratio). 
// The antenna is designed to resonate at 500kHz and so can be tuned with the
// following setting. The accuracy of the antenna must be within 3.5 percent of
// that value for proper signal validation and distance estimation.
void SparkFun_AS3935::changeDivRatio(uint8_t _divisionRatio)
{
  if( (_divisionRatio == 16) || (_divisionRatio == 32) || 
      (_divisionRatio == 64) || (_divisionRatio == 128) ) { }
  else
    return;

  if(_divisionRatio == 16) 
    writeRegister(INT_MASK_ANT, ((1<<7)|(1<<6)), 0, 6); //Demonstrative
  if(_divisionRatio == 32) 
    writeRegister(INT_MASK_ANT, ((1<<7)|(1<<6)), 1, 6); 
  else if(_divisionRatio == 64) 
    writeRegister(INT_MASK_ANT, ((1<<7)|(1<<6)), 1, 7); 
  else if(_divisionRatio == 128) 
    writeRegister(INT_MASK_ANT, ((1<<7)|(1<<6)), ((1<<7)|(1<<6)), 6); 
}

// REG0x03, bit [7:6], manufacturer default: 0 (16 division ratio). 
// This function returns the current division ratio of the resonance frequency.
// The antenna resonance frequency should be within 3.5 percent of 500kHz, and
// so when modifying the resonance frequency with the internal capacitors
// (tuneCap()) it's important to keep in mind that the displayed frequency on
// the IRQ pin is divided by this number. 
uint8_t SparkFun_AS3935::readDivisionRatio(){

  uint8_t regVal = readRegister(INT_MASK_ANT, 1); 
  regVal &= (~DIV_MASK); //I only want to return the two bits on the big endian side

  // The bits don't translate directly into the division ratio, and so here
  // we're checking the value in order to return the correct corresponding value. 
  if( regVal == 0 )
    return 16; 
  else if( regVal == 0x40 ) 
    return 32;
  else if( regVal == 0x80 )
    return 64;
  else 
    return 128; 

}
// REG0x07, bit [5:0], manufacturer default: 0. 
// This register holds the distance to the front of the storm and not the
// distance to a lightning strike.  
uint8_t SparkFun_AS3935::distanceToStorm()
{
  uint8_t _dist = readRegister(DISTANCE, 1); 
  _dist &= (~DISTANCE_MASK); 
  return(_dist); 
}
// REG0x08, bits [5,6,7], manufacturer default: 0. 
// This will send the frequency of the oscillators to the IRQ pin. 
//  _osc 1, bit[5] = TRCO - System RCO at 32.768kHz
//  _osc 2, bit[6] = SRCO - Timer RCO Oscillators 1.1MHz
//  _osc 3, bit[7] = LCO - Frequency of the Antenna
void SparkFun_AS3935::displayOscillator(bool _state, int _osc)
{
  if( (_osc < 1) || (_osc > 3) )
    return;

  if(_state == true){
    if(_osc == 1)
      writeRegister(FREQ_DISP_IRQ, OSC_MASK, 1, 5); 
    if(_osc == 2)
      writeRegister(FREQ_DISP_IRQ, OSC_MASK, 1, 6); 
    if(_osc == 3)
      writeRegister(FREQ_DISP_IRQ, OSC_MASK, 1, 7); 
  }

  if(_state == false){
    if(_osc == 1)
      writeRegister(FREQ_DISP_IRQ, OSC_MASK, 0, 5); //Demonstrative
    if(_osc == 2)
      writeRegister(FREQ_DISP_IRQ, OSC_MASK, 0, 6); 
    if(_osc == 3)
      writeRegister(FREQ_DISP_IRQ, OSC_MASK, 0, 7); 
  }

}

// REG0x08, bits [3:0], manufacturer default: 0. 
// This setting will add capacitance to the series RLC antenna on the product
// to help tune its resonance. The datasheet specifies being within 3.5 percent
// of 500kHz to get optimal lightning detection and distance sensing.  
// It's possible to add up to 120pF in steps of 8pF to the antenna. 
void SparkFun_AS3935::tuneCap(uint8_t _farad)
{
  if (_farad > 15)
   return;

  writeRegister(FREQ_DISP_IRQ, CAP_MASK, _farad, 0);    
}

// LSB =  REG0x04, bits[7:0]
// MSB =  REG0x05, bits[7:0]
// MMSB = REG0x06, bits[4:0]
// This returns a 20 bit value that is the 'energy' of the lightning strike.
// According to the datasheet this is only a pure value that doesn't have any
// physical meaning. 
uint32_t SparkFun_AS3935::lightningEnergy()
{
  _tempPE = readRegister(ENERGY_LIGHT_MMSB, 1);
  _tempPE &= (~ENERGY_MASK); //Only interested in the first four bits. 
  // Temporary Value is large enough to handle a shift of 16 bits.
  _pureLight = _tempPE << 16; 
  _tempPE = readRegister(ENERGY_LIGHT_MSB, 1);
  // Temporary value is large enough to handle a shift of 8 bits.
  _pureLight |= _tempPE << 8; 
  // No shift here, directly OR'ed into _pureLight variable.
  _pureLight |= readRegister(ENERGY_LIGHT_LSB, 1);
  return _pureLight;
}
  
// This function handles all I2C write commands. It takes the register to write
// to, then will mask the part of the register that coincides with the
// given register, and then write the given bits to the register starting at
// the given start position.  
void SparkFun_AS3935::writeRegister(uint8_t _wReg, uint8_t _mask, uint8_t _bits, uint8_t _startPosition)
{
  if(_i2cPort == NULL) {
    _spiWrite = readRegister(_wReg, 1); // Get the current value of the register
    _spiWrite &= (~_mask); // Mask the position we want to write to
    _spiWrite |= (_bits << _startPosition); // Write the given bits to the variable
    _spiPort->beginTransaction(SPISettings(_spiPortSpeed, MSBFIRST, SPI_MODE1)); 
    digitalWrite(_cs, LOW); // Start communication
    _spiPort->transfer(_wReg); // Start write command at given register
    _spiPort->transfer(_spiWrite); // Write to register
    digitalWrite(_cs, HIGH); // End communcation
    _spiPort->endTransaction();
  }
  else { 
    _i2cWrite = readRegister(_wReg, 1); // Get the current value of the register
    _i2cWrite &= (~_mask); // Mask the position we want to write to.
    _i2cWrite |= (_bits << _startPosition);  // Write the given bits to the variable
    _i2cPort->beginTransmission(_address); // Start communication.
    _i2cPort->write(_wReg); // at register....
    _i2cPort->write(_i2cWrite); // Write register...
    _i2cPort->endTransmission(); // End communcation.
  }
}

// This function reads the given register. 
uint8_t SparkFun_AS3935::readRegister(uint8_t _reg, int _len)
{

  if(_i2cPort == NULL) {
    _spiPort->beginTransaction(SPISettings(_spiPortSpeed, MSBFIRST, SPI_MODE1)); 
    digitalWrite(_cs, LOW); // Start communication.
    _spiPort->transfer(_reg |= SPI_READ_M);  // Register OR'ed with SPI read command. 
    _regValue = _spiPort->transfer(0); // Get data from register.  
    // According to datsheet, the chip select must be written HIGH, LOW, HIGH
    // to correctly end the READ command. 
    digitalWrite(_cs, HIGH); 
    digitalWrite(_cs, LOW); 
    digitalWrite(_cs, HIGH); 
    _spiPort->endTransaction();
    return(_regValue); 
  }
  else {
    _i2cPort->beginTransmission(_address); 
    _i2cPort->write(_reg); // Moves pointer to register.
    _i2cPort->endTransmission(false); // 'False' here sends a restart message so that bus is not released
    _i2cPort->requestFrom(_address, _len); // Read the register.
    _regValue = _i2cPort->read();
    return(_regValue);
  }
}
