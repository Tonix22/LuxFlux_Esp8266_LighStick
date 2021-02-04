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

        const float accelToG  = 1.0f / ACCEL_BASE; // LSB per g   @ +/- 16g
        const float gyroToRad = DEG_TO_RAD / GYRO_BASE; // LSB per dps @ +/- 2000 deg/s

        void retrieve()
        {
            uint8_t buffer[14];
            MeasureBits data;
            int32_t* filter = &data.Abx;
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, &buffer[0], 14);
            
            data.Abx = (int16_t)((buffer[0] << 8) | buffer[1]);
            data.Aby = (int16_t)((buffer[2] << 8) | buffer[3]);
            data.Abz = (int16_t)((buffer[4] << 8) | buffer[5]);
            data.Gbx = (int16_t)((buffer[8] << 8) | buffer[9]);
            data.Gby = (int16_t)((buffer[10] << 8) | buffer[11]);
            data.Gbz = (int16_t)((buffer[12] << 8) | buffer[13]);
            data.Abx -= Offset.Abx; 
            data.Aby -= Offset.Aby; 
            data.Abz -= Offset.Abz; 
            data.Gbx -= Offset.Gbx; 
            data.Gby -= Offset.Gby; 
            data.Gbz -= Offset.Gbz;
            
            for(int i=0; i < 3;i++, filter++)
            {
                if(*filter < 700)
                {
                    *filter = 0;
                }
            }
            for(int i=0; i < 3;i++, filter++)
            {
                if(*filter < 500)
                {
                    *filter = 0;
                }
            }

            Ax = (float)(data.Abx*accelToG);
            Ay = (float)(data.Aby*accelToG);
            Az = (float)(data.Abz*accelToG);
            Gx = (float)(data.Gbx*gyroToRad);
            Gy = (float)(data.Gby*gyroToRad);
            Gz = (float)(data.Gbz*gyroToRad);

        }
    };
}

#endif

#endif 