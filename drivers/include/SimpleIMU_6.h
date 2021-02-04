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
            MeasureBits data;
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, &buffer[0], 14);
            data.Abx = (((buffer[0]) << 8) | buffer[1]);
            data.Aby = (((buffer[2]) << 8) | buffer[3]);
            data.Abz = (((buffer[4]) << 8) | buffer[5]);
            data.Gbx = (((buffer[8]) << 8) | buffer[9]);
            data.Gby = (((buffer[10]) << 8) | buffer[11]);
            data.Gbz = (((buffer[12]) << 8) | buffer[13]);

            Ax = (float)(data.Abx);
            Ay = (float)(data.Aby);
            Az = (float)(data.Abz);
            Gx = (float)(data.Gbx);
            Gy = (float)(data.Gby);
            Gz = (float)(data.Gbz);

        }
    };
}

#endif

#endif 