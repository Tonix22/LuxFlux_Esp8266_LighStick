#ifndef IMU6050_H
#define IMU6050_H

typedef enum
{
    IMU_CAL,
    IMU_END_CAL,
}imu_msgID;

void imu_init(void);

#endif