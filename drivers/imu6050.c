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



static const char *TAG = "main";
extern xQueueHandle Light_event;
xQueueHandle imu_event = NULL;
uint32_t P = 0;
int i = 0;

MessageID msg;
imu_msgID imu_msg;

MeasureBits RAW;
MeasureBits Offset;

char sensor_log[7][20] = {{"Acel x: "},{"Acel y: "},{"Acel z: "},{"TEMP: "},{"Gyro x: "},{"Gyro y: "},{"Gyro z: "}};

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

#define CALIB_MAX       25
#define SENSORS_NUM     7
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

/*static void i2c_task_example(void *arg)
{
    uint8_t sensor_data[14];
    uint8_t who_am_i, i;
    double Temp;
    static uint16_t error_count = 0;
    int ret;
    uint32_t IMU_read = 0;
    uint32_t gravity = 0;

    uint32_t couter= 0; 


    i2c_example_master_mpu6050_init(I2C_EXAMPLE_MASTER_NUM);

    while (1) {

        who_am_i = 0;
        i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, WHO_AM_I, &who_am_i, 1);

        if (0x68 != who_am_i) {
            error_count++;
        }

        memset(sensor_data, 0, 14);
        ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, sensor_data, 14);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "*******************\n");
            ESP_LOGI(TAG, "WHO_AM_I: 0x%02x\n", who_am_i);
            Temp = 36.53 + ((double)(int16_t)((sensor_data[6] << 8) | sensor_data[7]) / 340);
            ESP_LOGI(TAG, "TEMP: %d.%d\n", (uint16_t)Temp, (uint16_t)(Temp * 100) % 100);
            if(couter % 23 == 0)
            {
                for (i = 0; i < 7; i++) 
                {
                    IMU_read = (int16_t)((sensor_data[i * 2] << 8) | sensor_data[i * 2 + 1]);
                    gravity = (IMU_read*2000)/32768;
                    ESP_LOGI(TAG, "sensor_data[%d]: %d\n", i, IMU_read);
                    ESP_LOGI(TAG, "gravity[%d]= %d.%d\n",i,gravity/1000, gravity%1000);
                }
            }
            ESP_LOGI(TAG, "error_count: %d\n", error_count);
        } else {
            ESP_LOGE(TAG, "No ack, sensor not connected...skip...\n");
        }
        couter++;
        vTaskDelay(100 / portTICK_RATE_MS);
    }

    i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}*/

void imu_calib_light(void *arg)
{
    imu_event = xQueueCreate(10, sizeof(uint32_t));

    printf("IMU calibration\r\n");
    for(;;)
    {
        msg = CALIBRATION;
        for(int cnt = 0; cnt < SENSORS_NUM; cnt++)
        {
            printf(" calibrating: %i \r\n", cnt);
            imu_CALIBRATION(cnt);
        }
        msg = END_CALIBRATION;
        for(int cnt = 0; cnt < 16; cnt++)
        {

            if(!(xQueueSend(Light_event, &msg, 0)))
            {
                printf(" message failed 2\r\n");
            }
            else
            {
                printf(" CALIB END %d\r\n", cnt);
                vTaskDelay(10/ portTICK_RATE_MS);
            }
            vTaskDelay(10/ portTICK_RATE_MS);
            xQueueReceive(imu_event, &imu_msg, portMAX_DELAY);
        }
        vTaskDelay(100/ portTICK_RATE_MS);
    }
}

uint32_t imu_avg(uint32_t data)
{ 
    static int i = 0;
    static int P = 0;
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

void imu_CALIBRATION(int sensor_num)
{
    uint8_t who_am_i; //IMU identity and bit counter
    uint8_t sensor_data[14]; // array to save all sensors measurements
    static uint16_t error_count = 0;
    int ret; //auxiliar variable

    static uint32_t* RAWptr = &RAW.Abx;
    static uint32_t* Offsetptr = &Offset.Abx;

    uint8_t sensor_idx = (sensor_num*2);
    uint8_t sensor_address = 0x3B + sensor_idx;
    printf(" sen idx %d\r\n", sensor_idx);
    printf(" sen add %X\r\n", sensor_address);
     
    who_am_i = 0;
    i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, WHO_AM_I, &who_am_i, 1);

    if (0x68 != who_am_i) {
        error_count++;
    }
    memset(sensor_data, 0, 14);
    
    for(int cnt = 0; cnt < CALIB_MAX; cnt++)
    {
        if(!(xQueueSend(Light_event, &msg, 0)))
        {
            printf(" message failed 1 \r\n");
        }
        else
        {
            ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, sensor_address, &sensor_data[sensor_idx], 2);
            if(ret == ESP_OK)
            {
                *RAWptr = (int16_t)((sensor_data[sensor_idx] << 8) | sensor_data[sensor_idx+1]);
                *Offsetptr = imu_avg((*RAWptr)*1000);
            }
        }
        xQueueReceive(imu_event, &imu_msg, portMAX_DELAY);
        vTaskDelay(10 / portTICK_RATE_MS);
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

    /*
    if(c == 0)
    {
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            if(!(xQueueSend(Light_event, &msg, 0)))
            {
                printf(" message failed 1 \r\n");
            }
            else
            {
                ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_XOUT_H, &sensor_data[0], 2);
                if(ret == ESP_OK)
                {
                    RAW.Abx = (int16_t)((sensor_data[0] << 8) | sensor_data[1]);
                    Offset.Abx = imu_avg(RAW.Abx*1000);
                }
            }
            xQueueReceive(imu_event, &imu_msg, portMAX_DELAY);
            vTaskDelay(10 / portTICK_RATE_MS);
        }
        Offset.Abx/=1000;
        printf("ACEL offset X: %d \r\n", Offset.Abx);
        imu_avg(0); //RESET AVG
    }
    if(c == 1)
    {
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_YOUT_H, &sensor_data[2], 2);
            if(ret == ESP_OK)
            {
                RAW.Aby = (int16_t)((sensor_data[2] << 8) | sensor_data[3]);
                Offset.Aby = imu_avg(RAW.Aby*1000);
            }
            //vTaskDelay(10 / portTICK_RATE_MS);
        }
        Offset.Aby/=1000;
        printf("ACEL offset Y: %d \r\n", Offset.Aby);
        imu_avg(0);
    }

    if(c == 2)
    {
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, ACCEL_ZOUT_H, &sensor_data[4], 2);
            if(ret == ESP_OK)
            {
                RAW.Abz = (int16_t)((sensor_data[4] << 8) | sensor_data[5]);
                Offset.Abz = imu_avg(RAW.Abz*1000);
            }
           // vTaskDelay(10 / portTICK_RATE_MS);
        }
        Offset.Abz/=1000;
        printf("ACEL offset Z: %d \r\n", Offset.Abz);
        imu_avg(0);
    }

    if(c == 3)
    {
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, GYRO_XOUT_H, &sensor_data[8], 2);
            if(ret == ESP_OK)
            {
                RAW.Gbx = (int16_t)((sensor_data[8] << 8) | sensor_data[9]);
                Offset.Gbx = imu_avg(RAW.Gbx*1000);
            }
            //vTaskDelay(10 / portTICK_RATE_MS);
        }
        Offset.Gbx/=1000;
        printf("GYRO offset X: %d \r\n", Offset.Gbx);
        imu_avg(0);
    }

    if(c == 4)
    {
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, GYRO_YOUT_H, &sensor_data[10], 2);
            if(ret == ESP_OK)
            {
                RAW.Gby = (int16_t)((sensor_data[10] << 8) | sensor_data[11]);
                Offset.Gby = imu_avg(RAW.Gby*1000);
            }
            //vTaskDelay(10 / portTICK_RATE_MS);
        }
        Offset.Gby/=1000;
        printf("GYRO offset Y: %d \r\n", Offset.Gby);
        imu_avg(0);
    }
    if(c == 5)
    {
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            ret = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, GYRO_ZOUT_H, &sensor_data[12], 2);
            if(ret == ESP_OK)
            {
                RAW.Gbz = (int16_t)((sensor_data[12] << 8) | sensor_data[13]);
                Offset.Gbz = imu_avg(RAW.Gbz*1000);
            }
            //vTaskDelay(10 / portTICK_RATE_MS);
        }
        Offset.Gbz/=1000;
        printf("GYRO offset Z: %d \r\n", Offset.Gbz);
        imu_avg(0);
    }*/
    //vTaskDelay(100 / portTICK_RATE_MS);
}

void imu_init(void)
{
  i2c_example_master_mpu6050_init(I2C_NUM_0);
}

