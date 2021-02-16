/* I2C

   This code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// =============================================================================
// INCLUDES
// =============================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS_wrapper.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "IO_driver.h"
#include "Light_effects.h"
#include "Menu.h"
#include "imu6050.h"
#include "SimpleIMU_6.h"

// =============================================================================
// QUEUES
// =============================================================================
extern xQueueHandle Light_event;
xQueueHandle imu_light_queue = NULL;
xQueueHandle imu_cntrl_queue = NULL;

/*lvaues for queues*/
Light_MessageID imu_to_led_msg;
IMU_msgID       imu_cntrl;

// =============================================================================
// EVENT GROUPS
// =============================================================================
EventGroupHandle_t calib_flags;
EventBits_t calib_status;
// =============================================================================
// STRUCTS AND POINTERS
// =============================================================================
MeasureBits Offset;
int32_t* Offsetptr = &Offset.Abx;
// =============================================================================
// DEBUG
// =============================================================================
char sensor_log[7][20] = {{"Acel x: "},{"Acel y: "},{"Acel z: "},{"TEMP: "},{"Gyro x: "},{"Gyro y: "},{"Gyro z: "}};


/**
 *   CODE BRIEF
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
esp_err_t i2c_example_master_mpu6050_read(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len)
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
    esp_err_t err = i2c_example_master_mpu6050_write(i2c_num, PWR_MGMT_1, &cmd_data, 1);
    if(err == ESP_OK)
    {
        cmd_data = 0x07;    // Set the SMPRT_DIV sample rate to 1kHz
        ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, SMPLRT_DIV, &cmd_data, 1));
        cmd_data = 0x06;    // Set the Low Pass Filter to 5Hz bandwidth
        ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, CONFIG, &cmd_data, 1));
        cmd_data = 0x03 << 3;     // Set the GYRO range to ± 2000 °/s 
        ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, GYRO_CONFIG, &cmd_data, 1));
        cmd_data = 0x03 << 3;    // Set the ACCEL range to ± 16g 
        ESP_ERROR_CHECK(i2c_example_master_mpu6050_write(i2c_num, ACCEL_CONFIG, &cmd_data, 1));
        xEventGroupSetBits(calib_flags, IMU_HAS_CONECTION);
    }
    else
    {
        xEventGroupSetBits(calib_flags, IMU_NOT_CONNECT);
    }
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
    calib_flags = xEventGroupCreate();
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
            case IMU_CIRCULAR_DRAW:
                calib_status = xEventGroupGetBits(calib_flags);
                if((calib_status & SUCESS_CALIB))
                {
                    printf("here draw a circule\r\n");
                    IMU_timer_call();
                }
                break;
            case IMU_LINEAR_DRAW:
                calib_status = xEventGroupGetBits(calib_flags);
                if((calib_status & SUCESS_CALIB))
                {
                    printf("here draw a line\r\n");
                }
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

    printf("IMU calibration start: \r\n");

    for(int cnt = 0; cnt < SENSORS_NUM; cnt++)
    {
        printf("Sensor num: %i \r\n", cnt);
        calibration_status = calibrate_sensor(cnt);

        if(calibration_status != imu_ok)
        {
            break;
        }
    }
    if(calibration_status == imu_ok)
    {
        for(int cnt = 0; cnt < CALIB_END; cnt++)
        {
            imu_to_led_msg = END_CALIBRATION;
            if(!(xQueueSend(Light_event, &imu_to_led_msg, 0)))
            {
                printf(" message failed 2\r\n");
                break;
            }
            vTaskDelay(20/ portTICK_RATE_MS);
            xQueueReceive(imu_light_queue, &imu_to_led_msg, portMAX_DELAY);
        }
        imu_to_led_msg = OFF;
        xQueueSend(Light_event, &imu_to_led_msg, 0);
        xEventGroupSetBits(calib_flags, SUCESS_CALIB);
    }
    else
    {
        if(calibration_status == imu_err || 
           calibration_status == imu_I_dont_know_who_am_i)
        {
            printf("CHECK IMU CONECTION PLEASE OR RETRY CALIBRATION\r\n");
            imu_to_led_msg = ABORT_CALIBRATION;
            xQueueSend(Light_event, &imu_to_led_msg, 0);
        }
        else
        {
            imu_to_led_msg = OFF;
            xQueueSend(Light_event, &imu_to_led_msg, 0);
        }
        xEventGroupSetBits(calib_flags, TERMINATED);
    }

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
    uint8_t who_am_i = 0; //IMU identity and bit counter
    uint8_t sensor_data[2] = {0,0}; // array to save all sensors measurements
    uint8_t sensor_address = ACCEL_XOUT_H + (sensor_num * 2);
    int16_t data_16bit     = 0;
    int status = 0; //auxiliar variable
    int32_t average = 0;
    i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, WHO_AM_I, &who_am_i, 1);
    //printf("who AM I: %x \r\n",who_am_i);
    //check if the conection is correct
    //printf("sensor addres: %X\r\n",sensor_address);
    if (0x68 == who_am_i) 
    {
        xEventGroupSetBits(calib_flags, INPROGRESS_CALIB);
        for(int cnt = 0; cnt < CALIB_MAX; cnt++)
        {
            imu_to_led_msg = CALIBRATION;
            vTaskDelay(10 / portTICK_RATE_MS);
            
            if(!(xQueueSend(Light_event, &imu_to_led_msg, 0)))
            {
                printf(" message failed 1 \r\n");
            }
            else
            {
                status = i2c_example_master_mpu6050_read(I2C_EXAMPLE_MASTER_NUM, sensor_address, &sensor_data[0], 2);
                if(status == ESP_OK)
                {
                    data_16bit = (int16_t)((sensor_data[0] << 8) | sensor_data[1]);
                    if(cnt == 0)
                    {
                        average = data_16bit*1000;
                    }
                    else
                    {
                        average = (cnt*average + data_16bit*1000)/(cnt+1);
                    }
                }
                else
                {
                    printf("Error in: %s %d \r\n", sensor_log[sensor_num],*Offsetptr);
                    return imu_err;
                }
            }
            xQueueReceive(imu_light_queue, &imu_to_led_msg, portMAX_DELAY);//IMU_ACK light end

            calib_status = xEventGroupGetBits(calib_flags);

            if(calib_status & ABORT_CALIB)
            {
                xEventGroupClearBits(calib_flags,ABORT_CALIB);
                xEventGroupSetBits(calib_flags,TERMINATED);
                return imu_abort;
            }
        }
        average /=1000;

        (*Offsetptr) = average;
        printf("%s %d \r\n", sensor_log[sensor_num],*Offsetptr);
        

        if(Offsetptr != &Offset.Gbz)
        {
            Offsetptr++;
        }
        else
        {
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
