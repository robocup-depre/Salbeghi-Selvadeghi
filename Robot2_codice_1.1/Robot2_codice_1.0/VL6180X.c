
#define F_CPU 16000000ul

//#include <util/twi.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "VL6180X.h"
#include "I2C.h"
#include "serial.h"
// Defines /////////////////////////////////////////////////////////////////////

// The Arduino two-wire interface uses a 7-bit number for the address,
// and sets the last bit correctly based on reads and writes
#define ADDRESS_DEFAULT 0b01010010  //0x29

#define TW_WRITE 0
#define TW_READ 1

// RANGE_SCALER values for 1x, 2x, 3x scaling - see STSW-IMG003 core/src/vl6180x_api.c (ScalerLookUP[])
static uint16_t const ScalerValues[] = {0, 253, 127, 84};

// Constructors ////////////////////////////////////////////////////////////////
uint8_t address=ADDRESS_DEFAULT;
uint8_t scaling=0;
uint8_t ptp_offset=0;
uint16_t io_timeout=0;
bool did_timeout=false;

void setTimeout(uint16_t timeout) { io_timeout = timeout; }
uint16_t getTimeout(void) { return io_timeout; }
uint16_t readRangeContinuousMillimeters(void) { return (uint16_t)scaling * readRangeContinuous(); }
uint16_t readRangeSingleMillimeters(void) { return (uint16_t)scaling * readRangeSingle(); }
uint8_t getScaling(void) { return scaling; }


// Public Methods //////////////////////////////////////////////////////////////
void setVL6180X(){
//accensione dei singoli laser e cambiamento dell'indirizzo i2c
	DDRC=0xFF;
	PORTC=0;
	_delay_ms(250);
	//_delay_ms(1000);
	/*address=ADDRESS_DEFAULT;
	_delay_ms(500);
	PORTA|=(1<<PA0);
	_delay_ms(50);
	initialize();
	configureDefault();
	setAddress(0x20);
	setTimeout(500);
	*/
	address=ADDRESS_DEFAULT;
	PORTC|=(1<<PC1);
	_delay_ms(50);
	initialize();
	configureDefault();
	setAddress(0x22);
	setTimeout(500);
	
	address=ADDRESS_DEFAULT;
	PORTC|=(1<<PC2);
	_delay_ms(50);
	initialize();
	configureDefault();
	setAddress(0x24);
	setTimeout(500);
	
	address=ADDRESS_DEFAULT;
	PORTC|=(1<<PC3);
	_delay_ms(50);
	initialize();
	configureDefault();
	setAddress(0x26);
	setTimeout(500);
	
	address=ADDRESS_DEFAULT;
	PORTC|=(1<<PC4);
	_delay_ms(50);
	initialize();
	configureDefault();
	setAddress(0x28);
	setTimeout(500);
	
	address=ADDRESS_DEFAULT;
	PORTC|=(1<<PC5);
	_delay_ms(50);
	initialize();
	configureDefault();
	setAddress(0x30);
	setTimeout(500);
	
}

int readVL6180X(int sensore){
	
	switch (sensore) {
		case 1:
			address=(0x20<<1);
			break;
		case 2:
			address=(0x22<<1);
			break;
		case 3:
			address=(0x24<<1);
			break;
		case 4:
			address=(0x26<<1);
			break;
		case 5:
			address=(0x28<<1);
			break;
		case 6:
			address=(0x30<<1);
			break;
		default:
			return 999;
			break;
	}
	int misura=readRangeSingleMillimeters();
	address=ADDRESS_DEFAULT;
	return misura;
}



void setAddress(uint8_t new_addr)
{
  writeReg(I2C_SLAVE__DEVICE_ADDRESS, new_addr & 0x7F);
  address = new_addr<<1;
}

// Initialize sensor with settings from ST application note AN4545, section 9 -
// "Mandatory : private registers"
void initialize(void)
{
	
  // Store part-to-part range offset so it can be adjusted if scaling is changed
  ptp_offset = readReg(SYSRANGE__PART_TO_PART_RANGE_OFFSET);

  if (readReg(SYSTEM__FRESH_OUT_OF_RESET) == 1)
  {
    scaling = 1;

    writeReg(0x207, 0x01);
    writeReg(0x208, 0x01);
    writeReg(0x096, 0x00);
    writeReg(0x097, 0xFD); // RANGE_SCALER = 253
    writeReg(0x0E3, 0x00);
    writeReg(0x0E4, 0x04);
    writeReg(0x0E5, 0x02);
    writeReg(0x0E6, 0x01);
    writeReg(0x0E7, 0x03);
    writeReg(0x0F5, 0x02);
    writeReg(0x0D9, 0x05);
    writeReg(0x0DB, 0xCE);
    writeReg(0x0DC, 0x03);
    writeReg(0x0DD, 0xF8);
    writeReg(0x09F, 0x00);
    writeReg(0x0A3, 0x3C);
    writeReg(0x0B7, 0x00);
    writeReg(0x0BB, 0x3C);
    writeReg(0x0B2, 0x09);
    writeReg(0x0CA, 0x09);
    writeReg(0x198, 0x01);
    writeReg(0x1B0, 0x17);
    writeReg(0x1AD, 0x00);
    writeReg(0x0FF, 0x05);
    writeReg(0x100, 0x05);
    writeReg(0x199, 0x05);
    writeReg(0x1A6, 0x1B);
    writeReg(0x1AC, 0x3E);
    writeReg(0x1A7, 0x1F);
    writeReg(0x030, 0x00);

    writeReg(SYSTEM__FRESH_OUT_OF_RESET, 0);
  }
  else
  {
    // Sensor has already been initialized, so try to get scaling settings by
    // reading registers.

    uint16_t s = readReg16Bit(RANGE_SCALER);

    if      (s == ScalerValues[3]) { scaling = 3; }
    else if (s == ScalerValues[2]) { scaling = 2; }
    else                           { scaling = 1; }

    // Adjust the part-to-part range offset value read earlier to account for
    // existing scaling. If the sensor was already in 2x or 3x scaling mode,
    // precision will be lost calculating the original (1x) offset, but this can
    // be resolved by resetting the sensor and Arduino again.
    ptp_offset *= scaling;
  }
}

// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// writeReg(SYSTEM__MODE_GPIO1, 0x10);
void configureDefault(void)
{
  // "Recommended : Public registers"

  // readout__averaging_sample_period = 48
  writeReg(READOUT__AVERAGING_SAMPLE_PERIOD, 0x30);

  // sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01 according to Table 14 in datasheet)
  writeReg(SYSALS__ANALOGUE_GAIN, 0x46);

  // sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature recalibration after every 255 range measurements)
  writeReg(SYSRANGE__VHV_REPEAT_RATE, 0xFF);

  // sysals__integration_period = 99 (100 ms)
  // AN4545 incorrectly recommends writing to register 0x040; 0x63 should go in the lower byte, which is register 0x041.
  writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 0x0063);

  // sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)
  writeReg(SYSRANGE__VHV_RECALIBRATE, 0x01);


  // "Optional: Public registers"

  // sysrange__intermeasurement_period = 9 (100 ms)
  writeReg(SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);

  // sysals__intermeasurement_period = 49 (500 ms)
  writeReg(SYSALS__INTERMEASUREMENT_PERIOD, 0x31);

  // als_int_mode = 4 (ALS new sample ready interrupt); range_int_mode = 4 (range new sample ready interrupt)
  writeReg(SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24);


  // Reset other settings to power-on defaults

  // sysrange__max_convergence_time = 49 (49 ms)
  writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 0x31);

  // disable interleaved mode
  writeReg(INTERLEAVED_MODE__ENABLE, 0);

  // reset range scaling factor to 1x
  setScaling(1);
}

// Writes an 8-bit register
void writeReg(uint16_t reg, uint8_t value)
{
  if (startI2C()) {error(); stop();}									//mi restituisce 1 se lo start NON è avvenuto correttamente
  if (sendI2C(address|TW_WRITE)) {error(); stop();}				////mi restituisce 1 se la trasmissione NON è avvenuta correttamente
  if (sendI2C(reg >> 8) & 0xff) {error(); stop();}
  if (sendI2C(reg & 0xff)) {error(); stop();}
  if (sendI2C(value)) {error(); stop();}
  stop();
  //_delay_ms(5);
}

// Writes a 16-bit register
void writeReg16Bit(uint16_t reg, uint16_t value)
{
  if (startI2C()) {error(); stop();}
  if (sendI2C(address|TW_WRITE)) {error(); stop();}
  if (sendI2C(reg >> 8) & 0xff) {error(); stop();}
  if (sendI2C(reg & 0xff)) {error(); stop();}
  if (sendI2C((value>>8) & 0xFF)) {error(); stop();}
  if (sendI2C((value) & 0xFF)) {error(); stop();}
  stop();
  //_delay_ms(5);
}

// Writes a 32-bit register
void writeReg32Bit(uint16_t reg, uint32_t value)
{
 if (startI2C()) {error(); stop();}
 if (sendI2C(address|TW_WRITE)) {error(); stop();}
 if (sendI2C(reg >> 8) & 0xff) {error(); stop();}
 if (sendI2C(reg & 0xff)) {error(); stop();}
 if (sendI2C((value>>24) & 0xFF)) {error(); stop();}
 if (sendI2C((value>>16) & 0xFF)) {error(); stop();}
 if (sendI2C((value>>8) & 0xFF)) {error(); stop();}
 if (sendI2C((value) & 0xFF)) {error(); stop();}
 stop();
 //_delay_ms(5);
}

// Reads an 8-bit register
uint8_t readReg(uint16_t reg)
{
  if (startI2C()) {error(); stop();}
  if (sendI2C(address|TW_WRITE)) {error(); stop();}
  if (sendI2C(reg >> 8) & 0xff) {error(); stop();}
  if (sendI2C(reg & 0xff)) {error(); stop();}
	  
  // stop();
  startI2C();
  if (sendI2C(address|TW_READ)) {error(); stop();}
  uint8_t value=readNACK();
  stop();
  //_delay_ms(5);
  if (value>>8) { error(); stop();}
	  
  return value;
  
}

// Reads a 16-bit register
uint16_t readReg16Bit(uint16_t reg)
{
 uint16_t val;
 if (startI2C()) {error(); stop();}
 if (sendI2C(address|TW_WRITE)) {error(); stop();}
 if (sendI2C(reg >> 8) & 0xff) {error(); stop();}
 if (sendI2C(reg & 0xff)) {error(); stop();}
 //stop();
 startI2C();
 if (sendI2C(address|TW_READ)) {error(); stop();}
 uint16_t value=readACK();
 if(value>>8){stop();error();return 0;}
 uint16_t value1=readNACK();
 stop();
 if(value1>>8){stop();error();return 0;}
 val=((value<<8)|value1);
 //_delay_ms(5);
 return val;
}

// Reads a 32-bit register
uint32_t readReg32Bit(uint16_t reg)
{
  uint32_t val;
  startI2C();
  if (checkSTART()) {error(); stop();}
  sendI2C(address|TW_WRITE);
  if (checkMTSLAACK()) {error(); stop();}
  if (sendI2C(reg >> 8) & 0xff) {error(); stop();}
  if (sendI2C(reg & 0xff)) {error(); stop();}
  if (checkDATAACK()) {error(); stop();}
  //stop();
  startI2C();
  if (sendI2C(address|TW_READ)) {error(); stop();}
  uint32_t value=readACK();
  if(value>>8){stop();error();return 0;}
  uint32_t value1=readACK();
  if(value1>>8){stop();error();return 0;}
  uint32_t value2=readACK();
  if (value2>>8){stop();error();return 0;}
  uint32_t value3=readNACK();
  if (value3>>8){stop();error();return 0;}
  stop();
  val=((value3<<24)|(value2<<16)|(value1<<8)|value);
  //_delay_ms(5);
  return val;
}

// Set range scaling factor. The sensor uses 1x scaling by default, giving range
// measurements in units of mm. Increasing the scaling to 2x or 3x makes it give
// raw values in units of 2 mm or 3 mm instead. In other words, a bigger scaling
// factor increases the sensor's potential maximum range but reduces its
// resolution.

int constrain(int x,int a, int b){
	
	if ((x>a) && (x<b))
	{
		return x;
	}else if (x<=a)
	{
		return a;
	}
	else if (x>=b)
	{
		return b;
	}
	return x;
}




// Implemented using ST's VL6180X API as a reference (STSW-IMG003); see
// VL6180x_UpscaleSetScaling() in vl6180x_api.c.
void setScaling(uint8_t new_scaling)
{
  uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

  // do nothing if scaling value is invalid
  if (new_scaling < 1 || new_scaling > 3) { return; }

  scaling = new_scaling;
  writeReg16Bit(RANGE_SCALER, ScalerValues[scaling]);

  // apply scaling on part-to-part offset
  writeReg(SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset / scaling);

  // apply scaling on CrossTalkValidHeight
  writeReg(SYSRANGE__CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / scaling);

  // This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

  // enable early convergence estimate only at 1x scaling
  uint8_t rce = readReg(SYSRANGE__RANGE_CHECK_ENABLES);
  writeReg(SYSRANGE__RANGE_CHECK_ENABLES, (rce & 0xFE) | (scaling == 1));
}

// Performs a single-shot ranging measurement
uint8_t readRangeSingle()
{
  writeReg(SYSRANGE__START, 0x01);
  return readRangeContinuous();
}

// Performs a single-shot ambient light measurement
uint16_t readAmbientSingle()
{
  writeReg(SYSALS__START, 0x01);
  return readAmbientContinuous();
}

// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void startRangeContinuous(uint16_t period)
{
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = constrain(period_reg, 0, 254);

  writeReg(SYSRANGE__INTERMEASUREMENT_PERIOD, period_reg);
  writeReg(SYSRANGE__START, 0x03);
}

// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void startAmbientContinuous(uint16_t period)
{
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = constrain(period_reg, 0, 254);

  writeReg(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
  writeReg(SYSALS__START, 0x03);
}

// Starts continuous interleaved measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified). In this mode, each
// ambient light measurement is immediately followed by a range measurement.
//
// The datasheet recommends using this mode instead of running "range and ALS
// continuous modes simultaneously (i.e. asynchronously)".
//
// The period must be greater than the time it takes to perform both
// measurements. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void startInterleavedContinuous(uint16_t period)
{
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = constrain(period_reg, 0, 254);

  writeReg(INTERLEAVED_MODE__ENABLE, 1);
  writeReg(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
  writeReg(SYSALS__START, 0x03);
}

// Stops continuous mode. This will actually start a single measurement of range
// and/or ambient light if continuous mode is not active, so it's a good idea to
// wait a few hundred ms after calling this function to let that complete
// before starting continuous mode again or taking a reading.
void stopContinuous()
{

  writeReg(SYSRANGE__START, 0x01);
  writeReg(SYSALS__START, 0x01);

  writeReg(INTERLEAVED_MODE__ENABLE, 0);
}

// Returns a range reading when continuous mode is activated
// (readRangeSingle() also calls this function after starting a single-shot
// range measurement)
uint8_t readRangeContinuous()
{
  //uint16_t millis_start = millis();
  while ((readReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x04) == 0)
  {
//     if (io_timeout > 0 && ((uint16_t)millis() - millis_start) > io_timeout)
//     {
//       did_timeout = true;
//       return 255;
//     }
  }

  uint8_t range = readReg(RESULT__RANGE_VAL);
  writeReg(SYSTEM__INTERRUPT_CLEAR, 0x01);

  return range;
}

// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
uint16_t readAmbientContinuous()
{
 // uint16_t millis_start = millis();
  while ((readReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x20) == 0)
  {
//     if (io_timeout > 0 && ((uint16_t)millis() - millis_start) > io_timeout)
//     {
//       did_timeout = true;
//       return 0;
//     }
  }

  uint16_t ambient = readReg16Bit(RESULT__ALS_VAL);
  writeReg(SYSTEM__INTERRUPT_CLEAR, 0x02);

  return ambient;
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool timeoutOccurred()
{
  bool tmp = did_timeout;
  did_timeout = false;
  return tmp;
}


