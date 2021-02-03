#ifndef SimpleIMU_6_H
#define SimpleIMU_6_H



#ifdef __cplusplus
    
extern "C" {
    #endif

    void IMU_timer_call();

    #ifdef __cplusplus

    #include "imu6050.h"

    #define ACCEL_BASE	2048.0f		// LSB per g   @ +/- 16g
    #define GYRO_BASE   16.375f		// LSB per dps @ +/- 2000 deg/s
    #define DEG_TO_RAD	(3.141592654f / 180.0f)

    

    extern MeasureBits Offset;
    

    class IMU_Sensor
    {
        public:

        float Ax;
        float Ay;
        float Az;
        float Gx; //RAW GYRO DATA
        float Gy;
        float Gz;

        const float accelToG  = 1.0f / ACCEL_BASE;
        const float gyroToRad = DEG_TO_RAD / GYRO_BASE;

        void retrieve()
        {

            uint8_t buffer[14];
        
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, &buffer[0], 14);
            Ax = (((int16_t)buffer[0]) << 8) | buffer[1];
            Ay = (((int16_t)buffer[2]) << 8) | buffer[3];
            Az = (((int16_t)buffer[4]) << 8) | buffer[5];
            Gx = (((int16_t)buffer[8]) << 8) | buffer[9];
            Gy = (((int16_t)buffer[10]) << 8) | buffer[11];
            Gz = (((int16_t)buffer[12]) << 8) | buffer[13];

            Ax = (Ax*-Offset.Abx)*accelToG;
            Ay = (Ay*-Offset.Aby)*accelToG;
            Az = (Az*-Offset.Abz)*accelToG;
            Gx = (Gx-Offset.Gbx)*gyroToRad;
            Gy = (Gy-Offset.Gby)*gyroToRad;
            Gz = (Gz-Offset.Gbz)*gyroToRad;

        }
    };
}

#endif

#endif 