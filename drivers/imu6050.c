/* I2C example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"
#include "IO_driver.h"
#include "Light_effects.h"
#include "imu6050.h"



//static const char *TAG = "main";
extern xQueueHandle Light_event;
xQueueHandle imu_light_queue = NULL;
xQueueHandle imu_cntrl_queue = NULL;

uint32_t P = 0;
int i = 0;

Light_MessageID imu_to_led_msg;
IMU_msgID       imu_cntrl;


MeasureBits RAW;
MeasureBits Offset;

char sensor_log[7][20] = {{"Acel x: "},{"Acel y: "},{"Acel z: "},{"TEMP: "},{"Gyro x: "},{"Gyro y: "},{"Gyro z: "}};
int32_t aux;

int32_t* RAWptr = &RAW.Abx;
int32_t* Offsetptr = &Offset.Abx;

/**
 * TEST CODE BRIEF
 *
 * This example will show you how to use I2C module by running two tasks on i2c bus:
 *
 * - read external i2c sensor, here we use a MPU6050 sensor for instance.
 * - Use one I2C port(master mode) to read or write the other I2C port(slave mode) on one ESP8266 chip.
 *
 * Pin assignment:
 *
 * - master:
 *    GPIO14 is assigned as the data signal of i2c master port
 *    GPIO2 is assigned as the clock signal of i2c master port
 *
 * Connection:
 *
 * - connect sda/scl of sensor with GPIO14/GPIO2
 * - no need to add external pull-up resistors, driver will enable internal pull-up resistors.
 *
 * Test items:
 *
 * - read the sensor data, if connected.
 */

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


/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_example_master_init(void)
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_D5_SDA;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = GPIO_D4_SCL;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}

/**
 * @brief test code to write mpu6050
 *
 * 1. send data
 * ___________________________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | write data_len byte + ack  | stop |
 * --------|---------------------------|-------------------------|----------------------------|------|
 *
 * @param i2c_num I2C port number
 * @param reg_address slave reg address
 * @param data data to send
 * @param data_len data length
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
static esp_err_t i2c_example_master_mpu6050_write(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU6050_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
    i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

/**
 * @brief test code to read mpu6050
 *
 * 1. send reg address
 * ______________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | stop |
 * --------|---------------------------|-------------------------|------|
 *
 * 2. read data
 * ___________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | read data_len byte + ack(last nack)  | stop |
 * --------|---------------------------|--------------------------------------|------|
 *
 * @param i2c_num I2C port number
 * @param reg_address slave reg address
 * @param data data to read
 * @param data_len data length
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
static esp_err_t i2c_example_master_mpu6050_read(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU6050_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        return ret;
    }

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MPU6050_SENSOR_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}
/**
 * @brief start IMU control by I2C
 * */
static esp_err_t i2c_example_master_mpu6050_init(i2c_port_t i2c_num)
{
    uint8_t cmd_data;
    vTaskDelay(100 / portTICK_RATE_MS);
    i2c_example_master_init();
    cmd_data = 0x00;    // reset mpu6050
    ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, PWR_MGMT_1, &cmd_data, 1));
    cmd_data = 0x07;    // Set the SMPRT_DIV sample rate to 1kHz
    ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, SMPLRT_DIV, &cmd_data, 1));
    cmd_data = 0x06;    // Set the Low Pass Filter to 5Hz bandwidth
    ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, CONFIG, &cmd_data, 1));
    cmd_data = 0x18;    // Set the GYRO range to ± 2000 °/s 
    ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, GYRO_CONFIG, &cmd_data, 1));
    cmd_data = 0x01;    // Set the ACCEL range to ± 2g 
    ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, ACCEL_CONFIG, &cmd_data, 1));
    return ESP_OK;
}
/**
 * @brief init imu task
 * */
void imu_init(void)
{
    xTaskCreate(imu_task, "imu_task", 2048, NULL, 10, NULL);
}
/**
 * @brief The task recieves queue to execute commands
 * 1. Task wait for a message 
 * 2. Excute the subtask related with message
 * */
void imu_task(void *arg)
{
    //init I2C driver
    i2c_example_master_mpu6050_init(I2C_NUM_0);
    /*imu light is for message light*/
    imu_light_queue = xQueueCreate(10, sizeof(uint32_t));
    /*imu cntrl for external comunication with IMU*/
    imu_cntrl_queue = xQueueCreate(10, sizeof(uint32_t));

    while (1) 
    {
        if(xQueueReceive(imu_cntrl_queue, &imu_cntrl, portMAX_DELAY))
        {
            switch (imu_cntrl)
            {
            case IMU_START_CALIBRATION:
                imu_calib_light();
                break;
            
            default:
                printf("INVALID IMU msg\r\n");
                break;
            }   
        }
    }
    i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}

/**
 * @brief imu_calib_light
 * if error ocurrs ABORT_CALIBRATION and check hardware.
 * */
void imu_calib_light(void)
{
    int calibration_status = imu_ok;
    imu_to_led_msg         = CALIBRATION;

    printf("IMU calibration start: \r\n");

    for(int cnt = 0; cnt < SENSORS_NUM; cnt++)
    {
        printf("Sensor num: %i \r\n", cnt);
        calibration_status = calibrate_sensor(cnt);

        if(calibration_status == imu_err)
        {
            break;
        }
    }
    if(calibration_status == imu_ok)
    {
        imu_to_led_msg = END_CALIBRATION;

        for(int cnt = 0; cnt < 16; cnt++)
        {
            if(!(xQueueSend(Light_event, &imu_to_led_msg, 0)))
            {
                printf(" message failed 2\r\n");
                break;
            }
            else
            {
                //printf(" CALIB END %d\r\n", cnt);
                vTaskDelay(10/ portTICK_RATE_MS);
            }
            vTaskDelay(10/ portTICK_RATE_MS);
            xQueueReceive(imu_light_queue, &imu_to_led_msg, portMAX_DELAY);
        }
    }
    else
    {
        printf("CHECK IMU CONECTION PLEASE OR RETRY CALIBRATION\r\n");
        imu_to_led_msg = ABORT_CALIBRATION;
        xQueueSend(Light_event, &imu_to_led_msg, 0);
    }

    vTaskDelay(100/ portTICK_RATE_MS);
}

/**
 * @brief Calibration procedure
 * @param sensor_num Acel x,Acel y, Acel z, TEMP, Gyro x ,Gyro y ,Gyro z
 *
 * @return
 *     - OK Success
 *     - imu_err
 *     - imu_I_dont_know_who_am_i
 * */
int calibrate_sensor(int sensor_num)
{
    uint8_t who_am_i; //IMU identity and bit counter
    uint8_t sensor_data[14]; // array to save all sensors measurements
    uint8_t sensor_idx     = (sensor_num*2);
    uint8_t sensor_address = ACCEL_XOUT_H + sensor_idx;
    
    int status = 0; //auxiliar variable

    who_am_i = 0;
    i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, WHO_AM_I, &who_am_i, 1);
    //printf("who AM I: %x \r\n",who_am_i);
    //check if the conection is correct
    if (0x68 == who_am_i) 
    {
        memset(sensor_data, 0, 14);
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            if(!(xQueueSend(Light_event, &imu_to_led_msg, 0)))
            {
                printf(" message failed 1 \r\n");
            }
            else
            {
                status = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, sensor_address, &sensor_data[sensor_idx], 2);
                if(status == ESP_OK)
                {
                    *RAWptr = (int16_t)((sensor_data[sensor_idx] << 8) | sensor_data[sensor_idx+1]);
                    *Offsetptr = imu_avg((*RAWptr)*1000);
                }
                else
                {
                    printf("Error in: %s %d \r\n", sensor_log[sensor_num],*Offsetptr);
                    return imu_err;
                }
            }
            xQueueReceive(imu_light_queue, &imu_to_led_msg, portMAX_DELAY);
            imu_to_led_msg = CALIBRATION;
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        (*Offsetptr)/=1000;
        printf("%s %d \r\n", sensor_log[sensor_num],*Offsetptr);
        imu_avg(0); //RESET AVG

        if(RAWptr != &RAW.Gbz)
        {
            RAWptr++;
            Offsetptr++;
        }
        else
        {
            RAWptr = &RAW.Abx;
            Offsetptr = &Offset.Abx;
        }
    }
    else
    {
        //Can't Read IMU sensor
        status = imu_I_dont_know_who_am_i;
    }

    return status;
}

/**
 * @brief average general math function
 * 
 * */
int32_t imu_avg(int32_t data)
{ 
    static int32_t i = 0;
    static int32_t P = 0;
    if (i == 0)
    {
        P = data; 
    }
    else
    {
        P = P*i;
        P = P+data;
        P = P/(i+1);
    }
    if(i == CALIB_MAX)
    {
        i = 0;
        P = 0;
    }
    i++;
    return P;
}


