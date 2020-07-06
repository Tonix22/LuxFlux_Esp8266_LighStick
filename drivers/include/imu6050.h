#ifndef IMU6050_H
#define IMU6050_H
#include <stdint.h>

#define CALIB_MAX       25
#define SENSORS_NUM     7
#define CALIB_END       82 // 5 second
typedef enum
{
    IMU_ENABLE,
    IMU_START_CALIBRATION,
    IMU_CALIBRATION,
    IMU_END_CALIBRATION,
    IMU_DISABLE
}IMU_msgID;

typedef enum
{
    imu_ok  = 0,
    imu_err = -1,
    imu_I_dont_know_who_am_i = -1,
}IMU_status;

typedef struct//RAW ACCELERATION DATA
{
    int32_t Abx;
    int32_t Aby;
    int32_t Abz;
    int32_t temp;
    int32_t Gbx; //RAW GYRO DATA
    int32_t Gby;
    int32_t Gbz;
}MeasureBits;

int32_t imu_avg(int32_t data);
void imu_calib_light(void);
void imu_task(void *arg);
void imu_init(void);
int calibrate_sensor(int sensor_num);
#endif