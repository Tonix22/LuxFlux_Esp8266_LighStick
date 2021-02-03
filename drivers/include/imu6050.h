#ifndef IMU6050_H
#define IMU6050_H
// =============================================================================
// includes
// =============================================================================

#include <stdint.h>
#include "FreeRTOS_wrapper.h"
#include "driver/i2c.h"

// =============================================================================
// EVENT GROUPS BITS DEFINES
// =============================================================================
#define INPROGRESS_CALIB  BIT0
#define ABORT_CALIB       BIT1 
#define SUCESS_CALIB      BIT2
#define TERMINATED        BIT3
#define IMU_HAS_CONECTION BIT5
#define IMU_NOT_CONNECT   BIT6
// =============================================================================
// DEFINES
// =============================================================================
#define CALIB_MAX       25
#define SENSORS_NUM     7
#define CALIB_END       82 // 5 second

//REGISTER 

#define I2C_EXAMPLE_MASTER_SCL_IO           2                /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           14               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define MPU6050_SENSOR_ADDR                 0x68             /*!< slave address for MPU6050 sensor */
#define MPU6050_CMD_START                   0x41             /*!< Command to set measure mode */
#define MPU6050_WHO_AM_I                    0x75             /*!< Command to read WHO_AM_I reg */
#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */

/**
 * Define the mpu6050 register address:
 */
#define SMPLRT_DIV      0x19
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_XOUT_H    0x3B
#define ACCEL_XOUT_L    0x3C
#define ACCEL_YOUT_H    0x3D
#define ACCEL_YOUT_L    0x3E
#define ACCEL_ZOUT_H    0x3F
#define ACCEL_ZOUT_L    0x40
#define TEMP_OUT_H      0x41
#define TEMP_OUT_L      0x42
#define GYRO_XOUT_H     0x43
#define GYRO_XOUT_L     0x44
#define GYRO_YOUT_H     0x45
#define GYRO_YOUT_L     0x46
#define GYRO_ZOUT_H     0x47
#define GYRO_ZOUT_L     0x48
#define PWR_MGMT_1      0x6B
#define WHO_AM_I        0x75  /*!< Command to read WHO_AM_I reg */


// =============================================================================
// TYPE DEFINITION
// =============================================================================
typedef enum
{
    IMU_ENABLE,
    IMU_ACK,
    IMU_START_CALIBRATION,
    IMU_CIRCULAR_DRAW,
    IMU_LINEAR_DRAW,
    IMU_DISABLE
}IMU_msgID;

typedef enum
{
    imu_ok  = 0,
    imu_err = -1,
    imu_abort = -2,
    imu_I_dont_know_who_am_i = -3,
}IMU_status;
// =============================================================================
// STRUCT DEFINITION
// =============================================================================
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

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

int32_t imu_avg(int32_t data);
void imu_calib_light(void);
void imu_task(void *arg);
void imu_init(void);
int calibrate_sensor(int sensor_num);
void Position(int32_t Ab);
esp_err_t i2c_example_master_mpu6050_read(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len);

#endif