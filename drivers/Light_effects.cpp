#include <stdio.h>
#include <vector>
#include <list>
#include "esp_system.h"
#include "Light_effects.h"
#include "imu6050.h"
#include "memory_admin.h"
#include "driver/hw_timer.h"

Light* LedStick;
extern EventGroupHandle_t Menu_status;


xQueueHandle Light_event = NULL;
extern xQueueHandle imu_light_queue;

static bool sound_released = true;
static bool first_beat = true;

void sound_action (void* arg );

static uint32_t beat_time = 40000;

void Light_task(void *arg)
{

    Light_MessageID msg;
    IMU_msgID imu_msg;

    LedStick         = new Light(8);
    Light_event      = xQueueCreate(10, sizeof(uint32_t));
    bool Calibrating = false;

    for(;;)
    {
        if (xQueueReceive(Light_event, &msg, portMAX_DELAY))
        {
            switch (msg)
            {   
            case SOUND:
                if(sound_released == true)
                {
                    sound_released = false;
                    hw_timer_init(sound_action, NULL);
                    hw_timer_alarm_us(beat_time, false);
                }
                break;
            case ABORT_CALIBRATION:
                for(uint8_t blink=0; blink < 10; blink++)
                {
                    Flash_color(255,0,0,30);
                }
                break;

            case CALIBRATION:
                if(Calibrating == false)
                {   
                    Calibrating = true;
                    LedStick->Fade_colors[0] = {255, 0  , 0};
                    LedStick->Fade_colors[1] = {255, 128, 0};
                }
                Fade_color();
                imu_msg = IMU_CALIBRATION;
                //printf("Fade\r\n");
                xQueueSend(imu_light_queue, &imu_msg, 100);
                break;

            case END_CALIBRATION:
                if(Calibrating == true)
                {
                    Calibrating = false;
                    LedStick->Fade_colors[0] = {0, 0  , 0};
                    LedStick->Fade_colors[1] = {0, 255, 0};
                }
                Fade_color();
                imu_msg = IMU_END_CALIBRATION;
                xQueueSend(imu_light_queue, &imu_msg, 100);
                break;
            case OFF:
                LedStick->Led_stick_off();
                vTaskDelay(10/ portTICK_RATE_MS);
                LedStick->Led_stick_off();
            default:
                break;
            }
        }
    }
}
void Ligth_init(void)
{
    xTaskCreate(Light_task, "Light_task", 4096, NULL, 10, NULL);
    init_flash_status_group();
}

void IDLE_HW_TIMER(void *arg)
{
    static int i = 0;
    
    for(auto& group: LedStick->seq_it->group)
    {
        for(i = 0; i < group.pixels; i++)
        {
            LedStick->Paint_LED(group.color);
        }
    }
    LedStick->seq_it++;
    if(LedStick->seq_it != LedStick->sequence.end())
    {
        hw_timer_alarm_us(LedStick->seq_it->time*1000, true);
    }
    else
    {
        xEventGroupSetBits(Menu_status,IDLE_BUFFER_END);
    }
}

EventBits_t IDLE_light()
{
    LedStick->seq_it = LedStick->sequence.begin();
    hw_timer_init(IDLE_HW_TIMER, NULL);
    hw_timer_alarm_us(LedStick->seq_it->time*1000, true);

    EventBits_t wait_buffer = xEventGroupWaitBits(Menu_status,
            ABORT|IDLE_BUFFER_END,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY); // wait until seq is finished
    /*Terminate frames*/
    hw_timer_deinit();
    LedStick->sequence.clear();
    xEventGroupClearBits(Menu_status,IDLE_BUFFER_END);
    
    if(wait_buffer & ABORT)
    {
        LedStick->Led_stick_off();
        clear_file_cursor();
    }
    return wait_buffer;
}

void sound_action (void* arg )
{
    
    if(first_beat)
    {
        first_beat = false;
        LedStick->seq_it = LedStick->sequence.begin();
    }
    beat_time = (LedStick->seq_it->time)*1000;
    
    LedStick->Fill_Led_stick(LedStick->seq_it->group.back().color);
    LedStick->seq_it++;

    if(LedStick->seq_it == LedStick->sequence.end())
    {
        first_beat = true;
    }
    sound_released = true;
    input_IO_enable_isr(GPIO_SDD2, GPIO_INTR_NEGEDGE);
    hw_timer_deinit();
}


void Fade_color(void)
{
    static uint8_t leds_num = 0;
    static auto it = LedStick->Fade_colors.begin();
    leds_num++;

    for(int j=0; j < leds_num ;j++)//paint ledstick
    {
        LedStick->Paint_LED(*it);
    }
    if(leds_num == LedStick->pixels) // change color
    {
        leds_num = 0;
        it++;//next color
        if(it == LedStick->Fade_colors.end())
        {
            it = LedStick->Fade_colors.begin();
        }
    }
}

void Flash_color(uint8_t R, uint8_t G, uint8_t B, int ms_rate)
{ 
    LedStick->Fill_Led_stick(R,G,B);

    vTaskDelay(ms_rate/ portTICK_RATE_MS);

    LedStick->Led_stick_off();

    vTaskDelay(ms_rate/ portTICK_RATE_MS); 
}
void Set_Frames_buffer(uint8_t frames)
{
    LedStick->max_frames = frames;
}
void Pixels_OFF()
{
    LedStick->Led_stick_off();
}

/*
void Pixel_rainbow_Fade(void)
{
    static uint8_t leds_num = 0;
    auto it = LedStick->Pixel_Rainbow.begin() + leds_num;
    leds_num++;
    for(int i = 0; i < leds_num; i++, it--)
    {
        LedStick->Paint_LED(*it);
    }
    if(leds_num == LedStick->pixels)
    {
        //std::rotate(LedStick->Pixel_Rainbow.begin(),LedStick->Pixel_Rainbow.begin()+1,LedStick->Pixel_Rainbow.end());
        vTaskDelay(80/ portTICK_RATE_MS);
        for(int i = 0; i < leds_num; i++, it--)
        {
            LedStick->Paint_LED(*it);
        }
        leds_num = 0;
    }
    vTaskDelay(80/ portTICK_RATE_MS);
}

void Pixel_rainbow(void)
{
   
    int red = 0;
    int green=0;
    int blue=0;

    for(red=0; red < 255; red++)
    {
        vTaskDelay(LedStick->rainbow_delay/ portTICK_RATE_MS);
        LedStick->Fill_Led_stick(red,green,blue);
    }
    for(green=0; green < 255; green++)
    {
        vTaskDelay(LedStick->rainbow_delay/ portTICK_RATE_MS);
        LedStick->Fill_Led_stick(red,green,blue);
    }
    for(red=255; red!=0; red--)
    {
        vTaskDelay(LedStick->rainbow_delay/ portTICK_RATE_MS);
        LedStick->Fill_Led_stick(red,green,blue);
    }
    for(blue=0; blue < 255; blue++)
    {
        vTaskDelay(LedStick->rainbow_delay/ portTICK_RATE_MS);
        LedStick->Fill_Led_stick(red,green,blue);
    }
    for(green=255; green!=0; green--)
    {
        vTaskDelay(LedStick->rainbow_delay/ portTICK_RATE_MS);
        LedStick->Fill_Led_stick(red,green,blue);
    }

    for(blue=255; blue!=0; blue--)
    {
        vTaskDelay(LedStick->rainbow_delay/ portTICK_RATE_MS);
        LedStick->Fill_Led_stick(red,green,blue);
    }
}
*/


