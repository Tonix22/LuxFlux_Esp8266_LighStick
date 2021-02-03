/*  
 *  Very Simple Complimentary Filter for the Invesense MPU-6050
 *  Phillip Schmidt
 *  v1.0, June 2015
 */


#include <Math3D.h>
#include <stdio.h>

#include "SimpleIMU_6.h"
#include "driver/hw_timer.h"

#define _DEGREES(x) (57.29578 * x)

Quat AttitudeEstimateQuat;
Vec3 correction_World;
Vec3 Accel_Body, Accel_World;
Vec3 GyroVec;

const Vec3 VERTICAL = {0.0f, 0.0f, 1.0f};  // vertical vector in the World frame
Vec3 correction_Body;
Quat incrementalRotation;
Vec3 YPR;
IMUMath calc;

// =============================================================================
// TIMING
// =============================================================================
int sampleRate = 400;
int _GyroClk   = 8000;
int _sampleRateDiv = (_GyroClk / sampleRate) - 1;
unsigned long samplePeriod = ((_sampleRateDiv + 1) * 1000000UL) / _GyroClk; // 2500
int dbg_cnt_print = 0;

// =============================================================================
// RAW DATA READ
// =============================================================================
IMU_Sensor MPU;

/* 
1. Join i2c bus
2. set I2C speed to 400k
3. Generate and store accel bias offsets
4. Generate and store gyro bias offsets
*/

void IMU_READ(void *arg)// Start Main Loop
{

	MPU.retrieve();
	GyroVec.x = MPU.Gx;	 GyroVec.y    =  MPU.Gy; GyroVec.z    =  MPU.Gz; // move gyro data to vector structure
	Accel_Body.x=MPU.Ax; Accel_Body.x =  MPU.Ay; Accel_Body.z =  MPU.Az; // move accel data to vector structure

	Accel_World = calc.Rotate(AttitudeEstimateQuat, Accel_Body); // rotate accel from body frame to world frame

	correction_World = calc.CrossProd(Accel_World, VERTICAL); // cross product to determine error

	correction_Body = calc.Rotate(correction_World, AttitudeEstimateQuat); // rotate correction vector to body frame

	GyroVec = calc.Sum(GyroVec, correction_Body);  // add correction vector to gyro data

	incrementalRotation = calc.Quaternion(GyroVec, samplePeriod);  // create incremental rotation quat

	AttitudeEstimateQuat = calc.Mul(incrementalRotation, AttitudeEstimateQuat);  // quaternion integration (rotation composting through multiplication)



	if(dbg_cnt_print == sampleRate)	
	{
		dbg_cnt_print = 0;
		// only display data 2x per second
		YPR = calc.YawPitchRoll(AttitudeEstimateQuat);
		printf("Yaw:  %.2f", _DEGREES(-YPR.x));   
		printf("Pitch: %.2f", _DEGREES(-YPR.y)); 
		printf("Roll: %.2f", _DEGREES(-YPR.z));

		//display(AttitudeEstimateQuat);
		//display(GyroVec);
		//display(AccelVec);
	}
	dbg_cnt_print++;
	hw_timer_alarm_us(samplePeriod, true);

} // Main Loop End


//POLL each 3 ms
void IMU_timer_call()
{
	/*init hw timer*/
    hw_timer_init(IMU_READ, NULL);
	hw_timer_alarm_us(samplePeriod,true);
}