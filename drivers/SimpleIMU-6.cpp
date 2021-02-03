/*  
 *  Very Simple Complimentary Filter for the Invesense MPU-6050
 *  Phillip Schmidt
 *  v1.0, June 2015
 */


#include <Math3D.h>
#include <stdio.h>

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
unsigned long samplePeriod;
/* 
1. Join i2c bus
2. set I2C speed to 400k
3. Generate and store accel bias offsets
4. Generate and store gyro bias offsets
*/

void IMU_READ() // Start Main Loop
{

	//if(RateLoopTimer.check())  // check if it is time for next sensor sample, 400Hz
	{
		
		//GyroVec    = Vector(MPU.gX, MPU.gY, MPU.gZ);	// move gyro data to vector structure
		//Accel_Body = Vector(MPU.aX, MPU.aY, MPU.aZ);	// move accel data to vector structure

		Accel_World = calc.Rotate(AttitudeEstimateQuat, Accel_Body); // rotate accel from body frame to world frame

		correction_World = calc.CrossProd(Accel_World, VERTICAL); // cross product to determine error

		correction_Body = calc.Rotate(correction_World, AttitudeEstimateQuat); // rotate correction vector to body frame

		GyroVec = calc.Sum(GyroVec, correction_Body);  // add correction vector to gyro data

		incrementalRotation = calc.Quaternion(GyroVec, samplePeriod);  // create incremental rotation quat

		AttitudeEstimateQuat = calc.Mul(incrementalRotation, AttitudeEstimateQuat);  // quaternion integration (rotation composting through multiplication)

	}
	
	//else if(MaintenanceLoop.check())	
	{
		// only display data 2x per second
		YPR = calc.YawPitchRoll(AttitudeEstimateQuat);
		printf("Yaw:  %.2f", _DEGREES(-YPR.x));   
		printf("Pitch: %.2f", _DEGREES(-YPR.y)); 
		printf("Roll: %.2f", _DEGREES(-YPR.z));


		//display(AttitudeEstimateQuat);
		//display(GyroVec);
		//display(AccelVec);
	}
	
} // Main Loop End
