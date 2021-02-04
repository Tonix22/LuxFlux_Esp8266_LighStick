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

            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, &buffer[0], 2);
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_YOUT_H, &buffer[2], 2);
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_ZOUT_H, &buffer[4], 2);
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, GYRO_XOUT_H, &buffer[8], 2);
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, GYRO_YOUT_H, &buffer[10], 2);
            i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, GYRO_ZOUT_H, &buffer[12], 2);
            
            data.Abx = (int16_t)((buffer[0] << 8) | buffer[1]);
            data.Aby = (int16_t)((buffer[2] << 8) | buffer[3]);
            data.Abz = (int16_t)((buffer[4] << 8) | buffer[5]);
            data.Gbx = (int16_t)((buffer[8] << 8) | buffer[9]);
            data.Gby = (int16_t)((buffer[10] << 8) | buffer[11]);
            data.Gbz = (int16_t)((buffer[12] << 8) | buffer[13]);
            printf("******\r\n");
            printf("integer \r\n");
            printf("MPU.Ax: %i\r\n",data.Abx);
            printf("MPU.Ay: %i\r\n",data.Aby);
            printf("MPU.Az: %i\r\n",data.Abz);
            printf("MPU.Gx: %i\r\n",data.Gbx);
            printf("MPU.Gy: %i\r\n",data.Gby);
            printf("MPU.Gz: %i\r\n",data.Gbz);
            printf("float\r\n");

            Ax = (float)(data.Abx);
            Ay = (float)(data.Aby);
            Az = (float)(data.Abz);
            Gx = (float)(data.Gbx);
            Gy = (float)(data.Gby);
            Gz = (float)(data.Gbz);
            printf("MPU.Ax: %i\r\n",(int)(Ax));
            printf("MPU.Ay: %i\r\n",(int)(Ay));
            printf("MPU.Az: %i\r\n",(int)(Az));
            printf("MPU.Gx: %i\r\n",(int)(Gx));
            printf("MPU.Gy: %i\r\n",(int)(Gy));
            printf("MPU.Gz: %i\r\n",(int)(Gz));
            printf("******\r\n");

        }
    };
}

#endif

#endif 