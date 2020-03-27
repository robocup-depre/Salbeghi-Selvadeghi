/*
 * laser.c
 *
 * Created: 27/10/2017 21:23:49
 *  Author: Matteo
 */ 

#include <stdbool.h>
#include "VL53L0.h"
#include "I2C.h"
#include "serial.h"
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
// Constructors ////////////////////////////////////////////////////////////////
uint8_t last_status53; // status of last I2C transmission
uint8_t slave=ADDRESS_DEFAULT;
uint16_t io_timeout53=0;
bool did_timeout53=false;
uint16_t timeout_start_ms53;
uint8_t stop_variable53; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
uint32_t measurement_timing_budget_us53;
// Public Methods //////////////////////////////////////////////////////////////

uint8_t getAddress(void) { return slave; }
void setTimeout53(uint16_t timeout) { io_timeout53 = timeout; }
uint16_t getTimeout53(void) { return io_timeout53; }
		
void attribute_laser (int attribute)//attribute può essere 1,2,3 long range, high accuracy, high speed
{
	if (attribute==1)
	{
		setSignalRateLimit(0.1);
		// increase laser pulse periods (defaults are 14 and 10 PCLKs)
		setVcselPulsePeriod(VcselPeriodPreRange, 18);
		setVcselPulsePeriod(VcselPeriodFinalRange, 14);
	} 
	else if(attribute==2)
		{
			setMeasurementTimingBudget(200000);
		}
		else
			{
				setMeasurementTimingBudget(20000);
			}
	
}

void setAddress53(uint8_t new_addr)
{
	writeReg53(I2C_SLAVE_DEVICE_ADDRESS, new_addr & 0x7F);//per cambiare l'indirizzo dei sensori
}

void setVL53L0()
{
	/*
	DDRC=0b11111101;//sensore Avanti Alto				AA
	init_laser(true);
	_delay_ms(5);
	attribute_laser(High_Speed);
	//attribute_laser(Long_Range);
	_delay_ms(5);
	setAddress(45);//1	mantenere 45
*/
	/*
	PORTA|=(1<<PA0);
	_delay_ms(50);
	init_laser(true);
	_delay_ms(5);
	
	setAddress53(0x28);
	setTimeout53(500);
	_delay_ms(5);
	*/
	slave=ADDRESS_DEFAULT;
	PORTC|=(1<<PC6);
	_delay_ms(50);
	init_laser(true);
	attribute_laser(High_Accuracy);
	setTimeout53(500);
	startContinuous(0);
	setAddress53(0x28);
	
}

//per leggere dai vari sensori
uint16_t readVL53L0(int which)//richiamare la funzione tra apici il nome del sensore 
{
	switch(which)
	{
		case 1:						//Avanti Alto			'AA'
		slave=(0x28<<1);
		break;
	}
	uint16_t val=readRangeSingleMillimeters53();
	slave=ADDRESS_DEFAULT;
	return (val<MAXL)? (val/1):0;//(val/10) in centimetri
}

bool init_laser(bool io_2v8)
{
	// VL53L0X_DataInit() begin

	// sensor uses 1V8 mode for I/O by default; switch to 2V8 mode if necessary
	if (io_2v8)
	{
		writeReg53(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, readReg53(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01); // set bit 0
	}

	// "Set I2C standard mode"
	writeReg53(0x88, 0x00);

	writeReg53(0x80, 0x01);
	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x00);
	stop_variable53 = readReg53(0x91);
	writeReg53(0x00, 0x01);
	writeReg53(0xFF, 0x00);
	writeReg53(0x80, 0x00);

	// disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
	writeReg53(MSRC_CONFIG_CONTROL, readReg53(MSRC_CONFIG_CONTROL) | 0x12);

	// set final range signal rate limit to 0.25 MCPS (million counts per second)
	setSignalRateLimit(0.25);

	writeReg53(SYSTEM_SEQUENCE_CONFIG, 0xFF);
	
	uint8_t spad_count;
	bool spad_type_is_aperture;
	if (!getSpadInfo(&spad_count, &spad_type_is_aperture)) { return false; }

	uint8_t ref_spad_map[6];
	readMulti53(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

	writeReg53(0xFF, 0x01);
	writeReg53(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
	writeReg53(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
	writeReg53(0xFF, 0x00);
	writeReg53(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

	uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
	uint8_t spads_enabled = 0;

	for (uint8_t i = 0; i < 48; i++)
	{
		if (i < first_spad_to_enable || spads_enabled == spad_count)
		{
			// This bit is lower than the first one that should be enabled, or
			// (reference_spad_count) bits have already been enabled, so zero this bit
			ref_spad_map[i / 8] &= ~(1 << (i % 8));
		}
		else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
		{
			spads_enabled++;
		}
	}

	writeMulti53(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x00);

	writeReg53(0xFF, 0x00);
	writeReg53(0x09, 0x00);
	writeReg53(0x10, 0x00);
	writeReg53(0x11, 0x00);

	writeReg53(0x24, 0x01);
	writeReg53(0x25, 0xFF);
	writeReg53(0x75, 0x00);

	writeReg53(0xFF, 0x01);
	writeReg53(0x4E, 0x2C);
	writeReg53(0x48, 0x00);
	writeReg53(0x30, 0x20);

	writeReg53(0xFF, 0x00);
	writeReg53(0x30, 0x09);
	writeReg53(0x54, 0x00);
	writeReg53(0x31, 0x04);
	writeReg53(0x32, 0x03);
	writeReg53(0x40, 0x83);
	writeReg53(0x46, 0x25);
	writeReg53(0x60, 0x00);
	writeReg53(0x27, 0x00);
	writeReg53(0x50, 0x06);
	writeReg53(0x51, 0x00);
	writeReg53(0x52, 0x96);
	writeReg53(0x56, 0x08);
	writeReg53(0x57, 0x30);
	writeReg53(0x61, 0x00);
	writeReg53(0x62, 0x00);
	writeReg53(0x64, 0x00);
	writeReg53(0x65, 0x00);
	writeReg53(0x66, 0xA0);

	writeReg53(0xFF, 0x01);
	writeReg53(0x22, 0x32);
	writeReg53(0x47, 0x14);
	writeReg53(0x49, 0xFF);
	writeReg53(0x4A, 0x00);

	writeReg53(0xFF, 0x00);
	writeReg53(0x7A, 0x0A);
	writeReg53(0x7B, 0x00);
	writeReg53(0x78, 0x21);

	writeReg53(0xFF, 0x01);
	writeReg53(0x23, 0x34);
	writeReg53(0x42, 0x00);
	writeReg53(0x44, 0xFF);
	writeReg53(0x45, 0x26);
	writeReg53(0x46, 0x05);
	writeReg53(0x40, 0x40);
	writeReg53(0x0E, 0x06);
	writeReg53(0x20, 0x1A);
	writeReg53(0x43, 0x40);

	writeReg53(0xFF, 0x00);
	writeReg53(0x34, 0x03);
	writeReg53(0x35, 0x44);

	writeReg53(0xFF, 0x01);
	writeReg53(0x31, 0x04);
	writeReg53(0x4B, 0x09);
	writeReg53(0x4C, 0x05);
	writeReg53(0x4D, 0x04);

	writeReg53(0xFF, 0x00);
	writeReg53(0x44, 0x00);
	writeReg53(0x45, 0x20);
	writeReg53(0x47, 0x08);
	writeReg53(0x48, 0x28);
	writeReg53(0x67, 0x00);
	writeReg53(0x70, 0x04);
	writeReg53(0x71, 0x01);
	writeReg53(0x72, 0xFE);
	writeReg53(0x76, 0x00);
	writeReg53(0x77, 0x00);

	writeReg53(0xFF, 0x01);
	writeReg53(0x0D, 0x01);

	writeReg53(0xFF, 0x00);
	writeReg53(0x80, 0x01);
	writeReg53(0x01, 0xF8);

	writeReg53(0xFF, 0x01);
	writeReg53(0x8E, 0x01);
	writeReg53(0x00, 0x01);
	writeReg53(0xFF, 0x00);
	writeReg53(0x80, 0x00);

	writeReg53(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
	writeReg53(GPIO_HV_MUX_ACTIVE_HIGH, readReg53(GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10); // active low
	writeReg53(SYSTEM_INTERRUPT_CLEAR, 0x01);

	measurement_timing_budget_us53 = getMeasurementTimingBudget();

	writeReg53(SYSTEM_SEQUENCE_CONFIG, 0xE8);

	setMeasurementTimingBudget(measurement_timing_budget_us53);

	writeReg53(SYSTEM_SEQUENCE_CONFIG, 0x01);
	if (!performSingleRefCalibration(0x40)) { return false; }

	writeReg53(SYSTEM_SEQUENCE_CONFIG, 0x02);
	if (!performSingleRefCalibration(0x00)) { return false; }

	writeReg53(SYSTEM_SEQUENCE_CONFIG, 0xE8);

	return true;
}

// Write an 8-bit register
void writeReg53(uint8_t reg, uint8_t value)
{
	if (startI2C()) {error(); stop();}									//mi restituisce 1 se lo start NON è avvenuto correttamente
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}				////mi restituisce 1 se la trasmissione NON è avvenuta correttamente
	if (sendI2C(reg)) {error(); stop();}
	if (sendI2C(value)) {error(); stop();}
	stop();
	_delay_ms(5);
}

// Write a 16-bit register
void writeReg16Bit53(uint8_t reg, uint16_t value)
{
	if (startI2C()) {error(); stop();}
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}
	if (sendI2C(reg)) {error(); stop();}
	if (sendI2C((value>>8) & 0xFF)) {error(); stop();}
	if (sendI2C((value) & 0xFF)) {error(); stop();}
	stop();
	_delay_ms(5);
}

// Write a 32-bit register
void writeReg32Bit53(uint8_t reg, uint32_t value)
{
	if (startI2C()) {error(); stop();}
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}
	if (sendI2C(reg)) {error(); stop();}
	if (sendI2C((value>>24) & 0xFF)) {error(); stop();}
	if (sendI2C((value>>16) & 0xFF)) {error(); stop();}
	if (sendI2C((value>>8) & 0xFF)) {error(); stop();}
	if (sendI2C((value) & 0xFF)) {error(); stop();}
	stop();
	_delay_ms(5);
}

// Read an 8-bit register
uint8_t readReg53(uint8_t reg)
{
	if (startI2C()) {error(); stop();}
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}
	if (sendI2C(reg)) {error(); stop();}
	//stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {error(); stop();}
	uint8_t value=readNACK();
	stop();
	_delay_ms(5);
	if (value>>8) { error(); stop();}
	return value;
}

// Read a 16-bit register
uint16_t readReg16Bit53(uint8_t reg)
{
	uint16_t val;
	if (startI2C()) {error(); stop();}
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}
	if (sendI2C(reg)) {error(); stop();}
	//stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {error(); stop();}
	uint16_t value=readACK();
	if(value>>8){stop();error();return 0;}
	uint16_t value1=readNACK();
	stop();
	if(value1>>8){stop();error();return 0;}
	val=((value<<8)|value1);
	_delay_ms(5);
	return val;
}

// Read a 32-bit register
uint32_t readReg32Bit53(uint8_t reg)
{
	uint32_t val;
	startI2C();
	if (checkSTART()) {error(); stop();}
	sendI2C(slave|TW_WRITE);
	if (checkMTSLAACK()) {error(); stop();}
	sendI2C(reg);
	if (checkDATAACK()) {error(); stop();}
	//stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {error(); stop();}
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
	_delay_ms(5);
	return val;
}

// Write an arbitrary number of bytes from the given array to the sensor,
// starting at the given register
void writeMulti53(uint8_t reg, uint8_t const * src, uint8_t count)//per settare in sequenza un registro che incrementa automaticamente
{
	if (startI2C()) {error(); stop();}
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}
	if (sendI2C(reg)) {error(); stop();}
		
	while (count > 0)
	{
		if (sendI2C(*(src++))) {error(); stop();}
		count--;
	}
	stop();
}

// Read an arbitrary number of bytes from the sensor, starting at the given
// register, into the given array
void readMulti53(uint8_t reg, uint8_t * dst, uint8_t count)//per leggere in sequenza un registro che incrementa automaticamente
{
	if (startI2C()) {error(); stop();}
	if (sendI2C(slave|TW_WRITE)) {error(); stop();}
	if (sendI2C(reg)) {error(); stop();}
	//stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {error(); stop();}
	for(int i=0;i<count;i++)
	{
		if(i<(count-1))
		{
			*(dst+i)=readACK();
		}
		else
		{
			*(dst+i)=readNACK();
		}
	}
	stop();
}


// Set the return signal rate limit check value in units of MCPS (mega counts
// per second). "This represents the amplitude of the signal reflected from the
// target and detected by the device"; setting this limit presumably determines
// the minimum measurement necessary for the sensor to report a valid reading.
// Setting a lower limit increases the potential range of the sensor but also
// seems to increase the likelihood of getting an inaccurate reading because of
// unwanted reflections from objects other than the intended target.
// Defaults to 0.25 MCPS as initialized by the ST API and this library.
bool setSignalRateLimit(float limit_Mcps)
{
	if (limit_Mcps < 0 || limit_Mcps > 511.99) { return false; }

	// Q9.7 fixed point format (9 integer bits, 7 fractional bits)
	writeReg16Bit53(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit_Mcps * (1 << 7));
	return true;
}

// Get the return signal rate limit check value in MCPS
float getSignalRateLimit(void)
{
	return (float)readReg16Bit53(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT) / (1 << 7);
}

// Set the measurement timing budget in microseconds, which is the time allowed
// for one measurement; the ST API and this library take care of splitting the
// timing budget among the sub-steps in the ranging sequence. A longer timing
// budget allows for more accurate measurements. Increasing the budget by a
// factor of N decreases the range measurement standard deviation by a factor of
// sqrt(N). Defaults to about 33 milliseconds; the minimum is 20 ms.
// based on VL53L0X_set_measurement_timing_budget_micro_seconds()
bool setMeasurementTimingBudget(uint32_t budget_us)
{
	SequenceStepEnables enables;
	SequenceStepTimeouts timeouts;

	uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
	uint16_t const EndOverhead        = 960;
	uint16_t const MsrcOverhead       = 660;
	uint16_t const TccOverhead        = 590;
	uint16_t const DssOverhead        = 690;
	uint16_t const PreRangeOverhead   = 660;
	uint16_t const FinalRangeOverhead = 550;

	uint32_t const MinTimingBudget = 20000;

	if (budget_us < MinTimingBudget) { return false; }

	uint32_t used_budget_us = StartOverhead + EndOverhead;

	getSequenceStepEnables(&enables);
	getSequenceStepTimeouts(&enables, &timeouts);

	if (enables.tcc)
	{
		used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
	}

	if (enables.dss)
	{
		used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
	}
	else if (enables.msrc)
	{
		used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
	}

	if (enables.pre_range)
	{
		used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
	}

	if (enables.final_range)
	{
		used_budget_us += FinalRangeOverhead;

		// "Note that the final range timeout is determined by the timing
		// budget and the sum of all other timeouts within the sequence.
		// If there is no room for the final range timeout, then an error
		// will be set. Otherwise the remaining time will be applied to
		// the final range."

		if (used_budget_us > budget_us)
		{
			// "Requested timeout too big."
			return false;
		}

		uint32_t final_range_timeout_us = budget_us - used_budget_us;

		// set_sequence_step_timeout() begin
		// (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

		// "For the final range timeout, the pre-range timeout
		//  must be added. To do this both final and pre-range
		//  timeouts must be expressed in macro periods MClks
		//  because they have different vcsel periods."

		uint16_t final_range_timeout_mclks =
		timeoutMicrosecondsToMclks(final_range_timeout_us,
		timeouts.final_range_vcsel_period_pclks);

		if (enables.pre_range)
		{
			final_range_timeout_mclks += timeouts.pre_range_mclks;
		}

		writeReg16Bit53(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
		encodeTimeout(final_range_timeout_mclks));

		// set_sequence_step_timeout() end

		measurement_timing_budget_us53 = budget_us; // store for internal reuse
	}
	return true;
}

// Get the measurement timing budget in microseconds
// based on VL53L0X_get_measurement_timing_budget_micro_seconds()
// in us
uint32_t getMeasurementTimingBudget(void)
{
	SequenceStepEnables enables;
	SequenceStepTimeouts timeouts;

	uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
	uint16_t const EndOverhead        = 960;
	uint16_t const MsrcOverhead       = 660;
	uint16_t const TccOverhead        = 590;
	uint16_t const DssOverhead        = 690;
	uint16_t const PreRangeOverhead   = 660;
	uint16_t const FinalRangeOverhead = 550;

	// "Start and end overhead times always present"
	uint32_t budget_us = StartOverhead + EndOverhead;

	getSequenceStepEnables(&enables);
	getSequenceStepTimeouts(&enables, &timeouts);

	if (enables.tcc)
	{
		budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
	}

	if (enables.dss)
	{
		budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
	}
	else if (enables.msrc)
	{
		budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
	}

	if (enables.pre_range)
	{
		budget_us += (timeouts.pre_range_us + PreRangeOverhead);
	}

	if (enables.final_range)
	{
		budget_us += (timeouts.final_range_us + FinalRangeOverhead);
	}

	measurement_timing_budget_us53 = budget_us; // store for internal reuse
	return budget_us;
}

// Set the VCSEL (vertical cavity surface emitting laser) pulse period for the
// given period type (pre-range or final range) to the given value in PCLKs.
// Longer periods seem to increase the potential range of the sensor.
// Valid values are (even numbers only):
//  pre:  12 to 18 (initialized default: 14)
//  final: 8 to 14 (initialized default: 10)
// based on VL53L0X_set_vcsel_pulse_period()
bool setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks)
{
	uint8_t vcsel_period_reg = encodeVcselPeriod(period_pclks);

	SequenceStepEnables enables;
	SequenceStepTimeouts timeouts;

	getSequenceStepEnables(&enables);
	getSequenceStepTimeouts(&enables, &timeouts);

	// "Apply specific settings for the requested clock period"
	// "Re-calculate and apply timeouts, in macro periods"

	// "When the VCSEL period for the pre or final range is changed,
	// the corresponding timeout must be read from the device using
	// the current VCSEL period, then the new VCSEL period can be
	// applied. The timeout then must be written back to the device
	// using the new VCSEL period.
	//
	// For the MSRC timeout, the same applies - this timeout being
	// dependant on the pre-range vcsel period."


	if (type == VcselPeriodPreRange)
	{
		// "Set phase check limits"
		switch (period_pclks)
		{
			case 12:
			writeReg53(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18);
			break;

			case 14:
			writeReg53(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30);
			break;

			case 16:
			writeReg53(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40);
			break;

			case 18:
			writeReg53(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50);
			break;

			default:
			// invalid period
			return false;
		}
		writeReg53(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);

		// apply new VCSEL period
		writeReg53(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

		// update timeouts

		// set_sequence_step_timeout() begin
		// (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)

		uint16_t new_pre_range_timeout_mclks =
		timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);

		writeReg16Bit53(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
		encodeTimeout(new_pre_range_timeout_mclks));

		// set_sequence_step_timeout() end

		// set_sequence_step_timeout() begin
		// (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)

		uint16_t new_msrc_timeout_mclks =
		timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);

		writeReg53(MSRC_CONFIG_TIMEOUT_MACROP,
		(new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1));

		// set_sequence_step_timeout() end
	}
	else if (type == VcselPeriodFinalRange)
	{
		switch (period_pclks)
		{
			case 8:
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
			writeReg53(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
			writeReg53(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
			writeReg53(0xFF, 0x01);
			writeReg53(ALGO_PHASECAL_LIM, 0x30);
			writeReg53(0xFF, 0x00);
			break;

			case 10:
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
			writeReg53(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			writeReg53(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
			writeReg53(0xFF, 0x01);
			writeReg53(ALGO_PHASECAL_LIM, 0x20);
			writeReg53(0xFF, 0x00);
			break;

			case 12:
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
			writeReg53(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			writeReg53(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
			writeReg53(0xFF, 0x01);
			writeReg53(ALGO_PHASECAL_LIM, 0x20);
			writeReg53(0xFF, 0x00);
			break;

			case 14:
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
			writeReg53(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
			writeReg53(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			writeReg53(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
			writeReg53(0xFF, 0x01);
			writeReg53(ALGO_PHASECAL_LIM, 0x20);
			writeReg53(0xFF, 0x00);
			break;

			default:
			// invalid period
			return false;
		}

		// apply new VCSEL period
		writeReg53(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

		// update timeouts

		// set_sequence_step_timeout() begin
		// (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

		// "For the final range timeout, the pre-range timeout
		//  must be added. To do this both final and pre-range
		//  timeouts must be expressed in macro periods MClks
		//  because they have different vcsel periods."

		uint16_t new_final_range_timeout_mclks =
		timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);

		if (enables.pre_range)
		{
			new_final_range_timeout_mclks += timeouts.pre_range_mclks;
		}

		writeReg16Bit53(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
		encodeTimeout(new_final_range_timeout_mclks));

		// set_sequence_step_timeout end
	}
	else
	{
		// invalid type
		return false;
	}

	// "Finally, the timing budget must be re-applied"

	setMeasurementTimingBudget(measurement_timing_budget_us53);

	// "Perform the phase calibration. This is needed after changing on vcsel period."
	// VL53L0X_perform_phase_calibration() begin

	uint8_t sequence_config = readReg53(SYSTEM_SEQUENCE_CONFIG);
	writeReg53(SYSTEM_SEQUENCE_CONFIG, 0x02);
	performSingleRefCalibration(0x0);
	writeReg53(SYSTEM_SEQUENCE_CONFIG, sequence_config);

	// VL53L0X_perform_phase_calibration() end

	return true;
}

// Get the VCSEL pulse period in PCLKs for the given period type.
// based on VL53L0X_get_vcsel_pulse_period()
uint8_t getVcselPulsePeriod(vcselPeriodType type)
{
	if (type == VcselPeriodPreRange)
	{
		return decodeVcselPeriod(readReg53(PRE_RANGE_CONFIG_VCSEL_PERIOD));
	}
	else if (type == VcselPeriodFinalRange)
	{
		return decodeVcselPeriod(readReg53(FINAL_RANGE_CONFIG_VCSEL_PERIOD));
	}
	else { return 255; }
}

// Start continuous ranging measurements. If period_ms (optional) is 0 or not
// given, continuous back-to-back mode is used (the sensor takes measurements as
// often as possible); otherwise, continuous timed mode is used, with the given
// inter-measurement period in milliseconds determining how often the sensor
// takes a measurement.
// based on VL53L0X_StartMeasurement()
void startContinuous(uint32_t period_ms)
{
	writeReg53(0x80, 0x01);
	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x00);
	writeReg53(0x91, stop_variable53);
	writeReg53(0x00, 0x01);
	writeReg53(0xFF, 0x00);
	writeReg53(0x80, 0x00);

	if (period_ms != 0)
	{
		// continuous timed mode

		// VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

		uint16_t osc_calibrate_val = readReg16Bit53(OSC_CALIBRATE_VAL);

		if (osc_calibrate_val != 0)
		{
			period_ms *= osc_calibrate_val;
		}

		writeReg32Bit53(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);

		// VL53L0X_SetInterMeasurementPeriodMilliSeconds() end

		writeReg53(SYSRANGE_START, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
	}
	else
	{
		// continuous back-to-back mode
		writeReg53(SYSRANGE_START, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
	}
}

// Stop continuous measurements
// based on VL53L0X_StopMeasurement()
void stopContinuous53(void)
{
	writeReg53(SYSRANGE_START, 0x01); // VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT

	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x00);
	writeReg53(0x91, 0x00);
	writeReg53(0x00, 0x01);
	writeReg53(0xFF, 0x00);
}

// Returns a range reading in millimeters when continuous mode is active
// (readRangeSingleMillimeters() also calls this function after starting a
// single-shot range measurement)
uint16_t readRangeContinuousMillimeters53(void)
{
	/*startTimeout();*/
	while ((readReg53(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
	{
// 		if (checkTimeoutExpired())
// 		{
// 			did_timeout = true;
// 			return 65535;
// 		}
	}

	// assumptions: Linearity Corrective Gain is 1000 (default);
	// fractional ranging is not enabled
	uint16_t range = readReg16Bit53(RESULT_RANGE_STATUS + 10);

	writeReg53(SYSTEM_INTERRUPT_CLEAR, 0x01);

	return range;
}

// Performs a single-shot range measurement and returns the reading in
// millimeters
// based on VL53L0X_PerformSingleRangingMeasurement()
uint16_t readRangeSingleMillimeters53(void)
{
	writeReg53(0x80, 0x01);
	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x00);
	writeReg53(0x91, stop_variable53);
	writeReg53(0x00, 0x01);
	writeReg53(0xFF, 0x00);
	writeReg53(0x80, 0x00);

	writeReg53(SYSRANGE_START, 0x01);

	// "Wait until start bit has been cleared"
	//startTimeout();
	while (readReg53(SYSRANGE_START) & 0x01)
	{
// 		if (checkTimeoutExpired())
// 		{
// 			did_timeout = true;
// 			return 65535;
// 		}
	}

	return readRangeContinuousMillimeters53();
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool timeoutOccurred53()
{
	bool tmp = did_timeout53;
	did_timeout53 = false;
	return tmp;
}

// Private Methods /////////////////////////////////////////////////////////////

// Get reference SPAD (single photon avalanche diode) count and type
// based on VL53L0X_get_info_from_device(),
// but only gets reference SPAD count and type
bool getSpadInfo(uint8_t * count, bool * type_is_aperture)
{
	uint8_t tmp;

	writeReg53(0x80, 0x01);
	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x00);

	writeReg53(0xFF, 0x06);
	writeReg53(0x83, readReg53(0x83) | 0x04);
	writeReg53(0xFF, 0x07);
	writeReg53(0x81, 0x01);

	writeReg53(0x80, 0x01);

	writeReg53(0x94, 0x6b);
	writeReg53(0x83, 0x00);
	//startTimeout();
	while (readReg53(0x83) == 0x00)
	{
		//if (checkTimeoutExpired()) { return false; }
	}
	writeReg53(0x83, 0x01);
	tmp = readReg53(0x92);

	*count = tmp & 0x7f;
	*type_is_aperture = (tmp >> 7) & 0x01;

	writeReg53(0x81, 0x00);
	writeReg53(0xFF, 0x06);
	writeReg53(0x83, readReg53(0x83)  & ~0x04);
	writeReg53(0xFF, 0x01);
	writeReg53(0x00, 0x01);

	writeReg53(0xFF, 0x00);
	writeReg53(0x80, 0x00);

	return true;
}

// Get sequence step enables
// based on VL53L0X_GetSequenceStepEnables()
void getSequenceStepEnables(SequenceStepEnables * enables)
{
	uint8_t sequence_config = readReg53(SYSTEM_SEQUENCE_CONFIG);

	enables->tcc          = (sequence_config >> 4) & 0x1;
	enables->dss          = (sequence_config >> 3) & 0x1;
	enables->msrc         = (sequence_config >> 2) & 0x1;
	enables->pre_range    = (sequence_config >> 6) & 0x1;
	enables->final_range  = (sequence_config >> 7) & 0x1;
}

// Get sequence step timeouts
// based on get_sequence_step_timeout(),
// but gets all timeouts instead of just the requested one, and also stores
// intermediate values
void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts)
{
	timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange);

	timeouts->msrc_dss_tcc_mclks = readReg53(MSRC_CONFIG_TIMEOUT_MACROP) + 1;
	timeouts->msrc_dss_tcc_us =
	timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
	timeouts->pre_range_vcsel_period_pclks);

	timeouts->pre_range_mclks =
	decodeTimeout(readReg16Bit53(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
	timeouts->pre_range_us =
	timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
	timeouts->pre_range_vcsel_period_pclks);

	timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange);

	timeouts->final_range_mclks =
	decodeTimeout(readReg16Bit53(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

	if (enables->pre_range)
	{
		timeouts->final_range_mclks -= timeouts->pre_range_mclks;
	}

	timeouts->final_range_us =
	timeoutMclksToMicroseconds(timeouts->final_range_mclks,
	timeouts->final_range_vcsel_period_pclks);
}

// Decode sequence step timeout in MCLKs from register value
// based on VL53L0X_decode_timeout()
// Note: the original function returned a uint32_t, but the return value is
// always stored in a uint16_t.
uint16_t decodeTimeout(uint16_t reg_val)
{
	// format: "(LSByte * 2^MSByte) + 1"
	return (uint16_t)((reg_val & 0x00FF) <<
	(uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
// Note: the original function took a uint16_t, but the argument passed to it
// is always a uint16_t.
uint16_t encodeTimeout(uint16_t timeout_mclks)
{
	// format: "(LSByte * 2^MSByte) + 1"

	uint32_t ls_byte = 0;
	uint16_t ms_byte = 0;

	if (timeout_mclks > 0)
	{
		ls_byte = timeout_mclks - 1;

		while ((ls_byte & 0xFFFFFF00) > 0)
		{
			ls_byte >>= 1;
			ms_byte++;
		}

		return (ms_byte << 8) | (ls_byte & 0xFF);
	}
	else { return 0; }
}

// Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_us()
uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

	return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_mclks()
uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

	return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}


// based on VL53L0X_perform_single_ref_calibration()
bool performSingleRefCalibration(uint8_t vhv_init_byte)
{
	writeReg53(SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

	//startTimeout();
	while ((readReg53(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
	{
		//if (checkTimeoutExpired()) { return false; }
	}

	writeReg53(SYSTEM_INTERRUPT_CLEAR, 0x01);

	writeReg53(SYSRANGE_START, 0x00);

	return true;
}
