#ifndef IMU6050_H
#define IMU6050_H
#include <stdint.h>
typedef enum
{
    IMU_CAL,
    IMU_END_CAL,
}imu_msgID;

typedef struct//RAW ACCELERATION DATA
{
    uint32_t Abx;
    uint32_t Aby;
    uint32_t Abz;
    uint32_t temp;
    uint32_t Gbx; //RAW GYRO DATA
    uint32_t Gby;
    uint32_t Gbz;
}MeasureBits;

void imu_init(void);
void imu_calib_light(void *arg);
void imu_CALIBRATION(int c);
#endif