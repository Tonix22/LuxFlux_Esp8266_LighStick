/*  
 *  Very Simple Complimentary Filter for the Invesense MPU-6050
 *  Phillip Schmidt
 *  v1.0, June 2015
 */

#include <vector>
#include <list>
#include <Math3D.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include "SimpleIMU_6.h"
#include "driver/hw_timer.h"
#include "Light_effects.h"

#define _DEGREES(x) (57.29578 * x)

extern xQueueHandle imu_light_queue;
extern Light* LedStick;
// =============================================================================
// QUATERNION AND VECTORS TO ESTIMATE POS
// =============================================================================

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
// LIGHT Circular
// =============================================================================

std::vector <CIRCLE_seg> Angle_by_color;
std::vector <CIRCLE_seg>::iterator it_color;
// =============================================================================
// TIMING
// =============================================================================

EventGroupHandle_t MPU_stat;
#define END_READ BIT0

int sampleRate = 400;
int _GyroClk   = 8000;
int _sampleRateDiv = (_GyroClk / sampleRate) - 1;
unsigned long samplePeriod = ((_sampleRateDiv + 1) * 1000000UL) / _GyroClk; // 2500
int refresh = 0;
int fps = 30;
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

	if(refresh == (sampleRate/fps))	
	{
		refresh = 0;
		xEventGroupSetBits(MPU_stat,END_READ);
	}else
	{
		refresh++;
		hw_timer_alarm_us(samplePeriod, true);
	}
	taskENABLE_INTERRUPTS(); // enable RTOS systic ISR

} // Main Loop End


//we divide a circule in N segments, range
//goes from -180 to 180, given the 360 circle
void Generate_Circular_Angles( std::list<Frame>& frames)
{
	std::list<Frame>::iterator seq_it = frames.begin();
	float b_sec = 0; // segment begin
	float e_sec = 0; // segment end
	int Segments = frames.size();
	float ratio  = 360/Segments;

	Angle_by_color.clear();
	Angle_by_color.reserve(Segments);
	it_color = Angle_by_color.begin();

	for(int i=0; i < Segments;i++)
	{
		e_sec +=ratio;
		if(e_sec > 180)
		{
			e_sec-=180;
		}
		Angle_by_color.emplace_back(b_sec,e_sec, &(*seq_it));
		b_sec = e_sec;
		seq_it++;
	}
}

//POLL each 3 ms
void IMU_timer_call()
{
	/*init hw timer*/

	EventBits_t wait_buffer;
	Light_MessageID led_to_imu;
    hw_timer_init(IMU_READ, NULL);
	MPU_stat = xEventGroupCreate();
	xQueueReceive(imu_light_queue, &led_to_imu, portMAX_DELAY);
	
	while(1)
	{
		hw_timer_alarm_us(samplePeriod,true);
		wait_buffer = xEventGroupWaitBits(MPU_stat,
		END_READ,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY); // wait until seq is finished
		
		xEventGroupClearBits(MPU_stat,END_READ);
		Evalutate_color_pos();
		//printf("****\r\n");
	}
	vEventGroupDelete(MPU_stat);

}

/*
std::vector::end
The past-the-end element is the theoretical element 
that would follow the last element in the vector. 
It does not point to any element, 
and thus shall not be dereferenced.*/

void Evalutate_color_pos()
{
	int YAW   ;
    int Pitch ;
    int Roll  ;
	YPR = calc.YawPitchRoll(AttitudeEstimateQuat);
	YAW   = (int)(_DEGREES(-YPR.x)*100);
	Pitch = (int)(_DEGREES(-YPR.y)*100);
	Roll  = (int)(_DEGREES(-YPR.y)*100);
	YAW /=100;

	if(it_color->end < YAW )
	{
		it_color++;
		if(it_color == Angle_by_color.end())
		{
			it_color = Angle_by_color.begin();
		}
	}
	else if(it_color->begin > YAW)
	{
		if(it_color == Angle_by_color.begin())
		{
			it_color = Angle_by_color.end();
		}
		it_color--;
	}
	for(auto& group: it_color->color->group)
    {
        for(int i = 0; i < group.pixels; i++)
        {
            LedStick->Paint_LED(group.color);
        }
    }
	//printf("Yaw:  %d.%d, ", YAW/100,    abs(YAW%100));   
	//printf("Pitch: %d.%d, ", Pitch/100, abs(Pitch%100)); 
	//printf("Roll:  %d.%d \r\n", Roll/100, abs(Roll%100));
}