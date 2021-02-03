/*  
 *  Very Simple Complimentary Filter for the Invesense MPU-6050
 *  Phillip Schmidt
 *  v1.0, June 2015
 */


#include <Math3D.h>


#define _DEGREES(x) (57.29578 * x)


PollTimer RateLoopTimer(400UL);
PollTimer MaintenanceLoop(2UL);


Quat AttitudeEstimateQuat;

Vec3 correction_Body, correction_World;
Vec3 Accel_Body, Accel_World;
Vec3 GyroVec;

const Vec3 VERTICAL = Vector(0.0f, 0.0f, 1.0f);  // vertical vector in the World frame
Vec3 correction_Body;
Quat incrementalRotation;
Vec3 YPR;
/* 
1. Join i2c bus
2. set I2C speed to 400k
3. Generate and store accel bias offsets
4. Generate and store gyro bias offsets

*/
void IMU_READ() // Start Main Loop
{

	if(RateLoopTimer.check())  // check if it is time for next sensor sample, 400Hz
	{
		GyroVec    = Vector(MPU.gX, MPU.gY, MPU.gZ);	// move gyro data to vector structure
		Accel_Body = Vector(MPU.aX, MPU.aY, MPU.aZ);	// move accel data to vector structure

		Accel_World = Rotate(AttitudeEstimateQuat, Accel_Body); // rotate accel from body frame to world frame

		correction_World = CrossProd(Accel_World, VERTICAL); // cross product to determine error

		correction_Body = Rotate(correction_World, AttitudeEstimateQuat); // rotate correction vector to body frame

		GyroVec = Sum(GyroVec, correction_Body);  // add correction vector to gyro data

		incrementalRotation = Quaternion(GyroVec, MPU.samplePeriod);  // create incremental rotation quat

		AttitudeEstimateQuat = Mul(incrementalRotation, AttitudeEstimateQuat);  // quaternion integration (rotation composting through multiplication)

	}
	//else if(MaintenanceLoop.check())	
	{
		// only display data 2x per second
		YPR = YawPitchRoll(AttitudeEstimateQuat);
		Serial.print("  Yaw:");   Serial.print(_DEGREES(-YPR.x), 2);
		Serial.print("  Pitch:"); Serial.print(_DEGREES(-YPR.y), 2);
		Serial.print("  Roll:");Serial.println(_DEGREES(-YPR.z), 2);


		//display(AttitudeEstimateQuat);
		//display(GyroVec);
		//display(AccelVec);
	}

} // Main Loop End
