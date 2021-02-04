/*  
 *  Very Simple Complimentary Filter for the Invesense MPU-6050
 *  Phillip Schmidt
 *  v1.0, June 2015
 */


#include <Math3D.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
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

EventGroupHandle_t MPU_stat;
#define END_READ BIT0
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
	taskDISABLE_INTERRUPTS();
	MPU.retrieve();

	GyroVec.x    = MPU.Gx;	 
	GyroVec.y    = MPU.Gy; 
	GyroVec.z    = MPU.Gz; // move gyro data to vector structure
	Accel_Body.x = MPU.Ax; 
	Accel_Body.y = MPU.Ay; 
	Accel_Body.z = MPU.Az; // move accel data to vector structure

	Accel_World = calc.Rotate(AttitudeEstimateQuat, Accel_Body); // rotate accel from body frame to world frame

	correction_World = calc.CrossProd(Accel_World, VERTICAL); // cross product to determine error

	correction_Body = calc.Rotate(correction_World, AttitudeEstimateQuat); // rotate correction vector to body frame

	GyroVec = calc.Sum(GyroVec, correction_Body);  // add correction vector to gyro data

	incrementalRotation = calc.Quaternion(GyroVec, samplePeriod+1800);  // create incremental rotation quat

	AttitudeEstimateQuat = calc.Mul(incrementalRotation, AttitudeEstimateQuat);  // quaternion integration (rotation composting through multiplication)

	if(dbg_cnt_print == (sampleRate))	
	{
		dbg_cnt_print = 0;

		xEventGroupSetBits(MPU_stat,END_READ);
	}else
	{
		dbg_cnt_print++;
		hw_timer_alarm_us(samplePeriod, true);
	}
	taskENABLE_INTERRUPTS(); // enable RTOS systic ISR

} // Main Loop End


//POLL each 3 ms
void IMU_timer_call()
{

	/*init hw timer*/
    hw_timer_init(IMU_READ, NULL);
	MPU_stat = xEventGroupCreate();
	EventBits_t wait_buffer;
	while(1)
	{
		hw_timer_alarm_us(samplePeriod,true);
		wait_buffer = xEventGroupWaitBits(MPU_stat,
		END_READ,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY); // wait until seq is finished
		
		xEventGroupClearBits(MPU_stat,END_READ);

		YPR = calc.YawPitchRoll(AttitudeEstimateQuat);
		int YAW   = (int)(_DEGREES(-YPR.x)*100);
		int Pitch = (int)(_DEGREES(-YPR.y)*100);
		int Roll  = (int)(_DEGREES(-YPR.y)*100);
		printf("Yaw:  %d.%d, ", YAW/100,    abs(YAW%100));   
		printf("Pitch: %d.%d, ", Pitch/100, abs(Pitch%100)); 
		printf("Roll:  %d.%d \r\n", Roll/100, abs(Roll%100));
		
		//printf("****\r\n");
	}
	vEventGroupDelete(MPU_stat);

}