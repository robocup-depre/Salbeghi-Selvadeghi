
 #include "BNO.h"
 #include "TWI.h"
 #include <util/atomic.h>
 #define BNO055_ADDRESS (0x29<<1)

 uint8_t GPwrMode = NormalG;    			// Gyro power mode
 uint8_t Gscale = GFS_2000DPS;  				// Gyro full scale
 //uint8_t Godr = GODR_250Hz;    // Gyro sample rate
 uint8_t Gbw = GBW_23Hz;       				// Gyro bandwidth
 uint8_t Ascale = AFS_4G;     			// Accel full scale
 //uint8_t Aodr = AODR_250Hz;    // Accel sample rate
 uint8_t APwrMode = NormalA;    			// Accel power mode
 uint8_t Abw = ABW_31_25Hz;   					// Accel bandwidth, accel sample rate divided by ABW_divx
 //uint8_t Mscale = MFS_4Gauss;  // Select magnetometer full-scale resolution
 uint8_t MOpMode = Regular;    			// Select magnetometer perfomance mode
 uint8_t MPwrMode = Normal;    			// Select magnetometer power mode
 uint8_t Modr = MODR_10Hz;     				// Select magnetometer ODR when in BNO055 bypass mode
 uint8_t PWRMode = Normalpwr;    			// Select BNO055 power mode
 uint8_t OPRMode = IMU;       			// specify operation mode for sensors
 uint8_t status;               	// BNO055 data status register

 //
 volatile bno055 BNO;

 void BNO_init()
 {
 TWI_send(BNO055_ADDRESS, BNO055_OPR_MODE, CONFIGMODE );
 _delay_ms(25);
 // Select page 1 to configure sensors
 TWI_send(BNO055_ADDRESS, BNO055_PAGE_ID, 0x01);
 // Configure ACC
 TWI_send(BNO055_ADDRESS, BNO055_ACC_CONFIG, APwrMode << 5 | Abw << 3 | Ascale );
 // Configure GYR
 TWI_send(BNO055_ADDRESS, BNO055_GYRO_CONFIG_0, Gbw << 3 | Gscale );
 TWI_send(BNO055_ADDRESS, BNO055_GYRO_CONFIG_1, GPwrMode);
 // Configure MAG
 TWI_send(BNO055_ADDRESS, BNO055_MAG_CONFIG, MPwrMode << 5 | MOpMode << 3 | Modr );
 // Select page 0 to read sensors
 TWI_send(BNO055_ADDRESS, BNO055_PAGE_ID, 0x00);
 // Select BNO055 gyro temperature source
 TWI_send(BNO055_ADDRESS, BNO055_TEMP_SOURCE, 0x01 );
 // Select BNO055 sensor units (temperature in degrees C, rate in dps, accel in mg)
 TWI_send(BNO055_ADDRESS, BNO055_UNIT_SEL, 0x01 );
 // Select BNO055 system power mode
 TWI_send(BNO055_ADDRESS, BNO055_PWR_MODE, PWRMode );
 // Select BNO055 system operation mode
 TWI_send(BNO055_ADDRESS, BNO055_OPR_MODE, OPRMode );
 _delay_ms(25);

 }
 uint8_t BNO_getCal(int what) {
	 uint8_t calData = TWI_read(BNO055_ADDRESS,BNO055_CALIB_STAT);
	 return (calData>>((3-what)*2)&0x03);
 }
float readAxis(int axis)
{
	int16_t temp;
	while(TWI_status(3));
	TWI_status(1);
switch(axis)
{
case 1:
temp= TWI_read2(BNO055_ADDRESS,BNO055_EUL_HEADING_LSB);
break;
case 2:
temp = TWI_read2(BNO055_ADDRESS,BNO055_EUL_HEADING_LSB+2);
break;
case 3:
temp = TWI_read2(BNO055_ADDRESS,BNO055_EUL_HEADING_LSB+4);
break;
default:
temp=6400;
break;
}	
TWI_status(0);
return (float)(temp/16.0);

}


float readGyroAngle(int axis){
	int16_t temp;
	while(TWI_status(3));
	TWI_status(1);
	switch(axis)
	{
		case 1:
		temp= TWI_read2(BNO055_ADDRESS,BNO055_GYR_DATA_X_LSB);
		break;
		case 2:
		temp = TWI_read2(BNO055_ADDRESS,BNO055_GYR_DATA_Y_LSB);
		break;
		case 3:
		temp = TWI_read2(BNO055_ADDRESS,BNO055_GYR_DATA_Z_LSB);
		break;
		default:
		temp=6400;
		break;
	}
	TWI_status(0);
	if(temp<0)temp*=(-1);
	return (float)(temp/16.0);
}

void readEul(){
	uint8_t rawData[6];  // x/y/z gyro register data stored here
	TWI_reads(BNO055_ADDRESS, BNO055_EUL_HEADING_LSB, 6, &rawData);  // Read the six raw data registers sequentially into data array
	int16_t intX = ((int16_t)rawData[1] << 8) | rawData[0] ;       // Turn the MSB and LSB into a signed 16-bit value
	int16_t intY = ((int16_t)rawData[3] << 8) | rawData[2] ;
	int16_t intZ = ((int16_t)rawData[5] << 8) | rawData[4] ;
	BNO.x= (float)intX/16.;
	BNO.y= (float)intY/16.;
	BNO.z= (float)intZ/16.;
}

void readQuat(){
	uint8_t rawData[8];  // x/y/z gyro register data stored here
	TWI_reads(BNO055_ADDRESS, BNO055_QUA_DATA_W_LSB, 8, &rawData);  // Read the six raw data registers sequentially into data array
	int16_t intQ0 = ((int16_t)rawData[1] << 8) | rawData[0] ;       // Turn the MSB and LSB into a signed 16-bit value
	int16_t intQ1 = ((int16_t)rawData[3] << 8) | rawData[2] ;
	int16_t intQ2 = ((int16_t)rawData[5] << 8) | rawData[4] ;
	int16_t intQ3 = ((int16_t)rawData[7] << 8) | rawData[6] ;
	BNO.q0= (float)intQ0/16384.;
	BNO.q1= (float)intQ1/16384.;
	BNO.q2= (float)intQ2/16384.;
	BNO.q3= (float)intQ3/16384.;

}

 float accelerazioneAngolare(int asse){
	 return readGyroAngle(asse);
 }
 float gradi_;
 float gradiGiroscopio(int asse){
	 	 gradi_= (asse==asseY)? init_asseY:0;
	 	 return (readAxis(asse)-gradi_);
 }