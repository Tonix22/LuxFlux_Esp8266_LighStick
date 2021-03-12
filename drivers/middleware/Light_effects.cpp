// =============================================================================
// C++ Includes
// =============================================================================
#include <stdio.h>
#include <vector>
#include <list>
#include "esp_system.h"
#include "Light_effects.h"
#include "imu6050.h"
#include "memory_admin.h"
#include "driver/hw_timer.h"
#include "tcp_server.h"

// =============================================================================
// Class
// =============================================================================

Light* LedStick;

// =============================================================================
// EventGroup
// =============================================================================

extern EventGroupHandle_t Menu_status;

// =============================================================================
// Queues
// =============================================================================

extern xQueueHandle imu_light_queue;
extern xQueueHandle tcp_light_event;
xQueueHandle Light_event = NULL;

// =============================================================================
// Local Variables
// =============================================================================

static bool sound_released = true;
static bool first_beat     = true;
static uint32_t beat_time  = 40000;

// =============================================================================
// Function prototypes
// =============================================================================
void sound_action (void* arg );
void Light_task   (void* arg );

// =============================================================================
// Class seters and getters
// =============================================================================
void Set_Frames_buffer(uint8_t frames)
{
    LedStick->max_frames = frames;
}

// =============================================================================
// Task
// =============================================================================

void Ligth_init(void)
{
    xTaskCreate(Light_task, "Light_task", 4096, NULL, 10, NULL);
    init_flash_status_group(); // Flash status flags allocate // TODO move to flash section
}

void Light_task(void* arg )
{
    /* Task comunication tools*/
    Light_MessageID light_queue;
    IMU_msgID imu_comm;
    TCP_msgID tcp_comm;

    // Locate resources
    LedStick         = new Light(8); ///8 pixels
    Light_event      = xQueueCreate(10, sizeof(uint32_t));

    bool load_calib_colors = false;

    for(;;)
    {
        if (xQueueReceive(Light_event, &light_queue, portMAX_DELAY))
        {
            switch (light_queue)
            {   
            case SOUND://This message is send from SOUND ISR
                if(sound_released == true)
                {
                    sound_released = false;
                    hw_timer_init(sound_action, NULL);
                    hw_timer_alarm_us(beat_time, false);
                }
                break;
            case ABORT_CALIBRATION: // Abort calibration Manualy
                for(uint8_t blink=0; blink < 10; blink++)
                {
                    Flash_color(255,0,0,30);
                }
                break;

            case CALIBRATION:  // report Calibrating status
                if(load_calib_colors == false)
                {   
                    load_calib_colors = true;
                    LedStick->Fade_colors[0] = {255, 0  , 0};
                    LedStick->Fade_colors[1] = {255, 128, 0};
                }
                Fade_color();
                imu_comm = IMU_ACK;
                //printf("Fade\r\n");
                xQueueSend(imu_light_queue, &imu_comm, 100);
                break;

            case END_CALIBRATION: // show end calibration sequence
                if(load_calib_colors == true)
                {
                    load_calib_colors = false;
                    LedStick->Fade_colors[0] = {0, 0  , 0};
                    LedStick->Fade_colors[1] = {0, 255, 0};
                }
                Fade_color();
                imu_comm = IMU_ACK;
                xQueueSend(imu_light_queue, &imu_comm, 100);
                break;
            case OFF: // turn off ledstick
                LedStick->Led_stick_off();
                vTaskDelay(10/ portTICK_RATE_MS);
                LedStick->Led_stick_off();
            case CIRC_LOAD:
                imu_comm = IMU_ACK;
                Generate_Circular_Angles(LedStick->sequence);
                xQueueSend(imu_light_queue, &imu_comm, 100);

                break;
            case TCP_LOAD:
                
                do
                {
                    Flash_color(0,0,255,500);
                    xQueueReceive(Light_event, &light_queue, 0);
                } while (light_queue!=OFF);  
                LedStick->Led_stick_off();
                break;
            case TCP_SYNC:

                break;
            

            default:
                break;
            }
        }
    }
}

// =============================================================================
// IDLE light sequence
// =============================================================================
/**
 * @brief
 * Hardware timer is used to give specific time perfomance between each frame.
 * RTOS task delay could provide lagg or not precise results
 * */
void IDLE_HW_TIMER(void *arg)
{
    static int i = 0;
    taskDISABLE_INTERRUPTS(); // STOP RTOS systic ISR
    for(auto& group: LedStick->seq_it->group)
    {
        for(i = 0; i < group.pixels; i++)
        {
            LedStick->Paint_LED(group.color);
        }
    }

    LedStick->seq_it++; // move to next Frame

    if(LedStick->seq_it != LedStick->sequence.end())//pending frames
    {
        
        hw_timer_alarm_us(ms_to_us(LedStick->seq_it->time), true);//Load next timer
    }
    else // buffer is over,go to load more frames
    {
        xEventGroupSetBits(Menu_status,IDLE_BUFFER_END); 
    }
    taskENABLE_INTERRUPTS(); // enable RTOS systic ISR
}
/**
 * @brief 
 * IDLE light secuence, load a personalized sequecence previously stored
 */
EventBits_t IDLE_light()
{
    LedStick->seq_it = LedStick->sequence.begin(); // point to frames beginig
    /*init hw timer*/
    hw_timer_init(IDLE_HW_TIMER, NULL);
    //hw_timer_alarm_us(ms_to_us(LedStick->seq_it->time), true);
    if(LedStick->seq_it != LedStick->sequence.end())//pending frames
    {
        //printf("TIME: %d \r\n",LedStick->seq_it->time);
        hw_timer_alarm_us(ms_to_us(LedStick->seq_it->time), true);//Load next timer
    }else
    {
        xEventGroupSetBits(Menu_status,IDLE_BUFFER_END);
    }
    /*Wait buffer is empty or abort signal request*/
    EventBits_t wait_buffer = xEventGroupWaitBits(Menu_status,
            ABORT|IDLE_BUFFER_END,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY); // wait until seq is finished


    /*Deallocate timer info*/
    hw_timer_deinit();
    /*Deallocate frames*/
    LedStick->sequence.clear();
    /*Clear buffer status*/
    xEventGroupClearBits(Menu_status,IDLE_BUFFER_END);

    return wait_buffer; // this flag to check if abort was called
}
// =============================================================================
// SOUND light sequence
// =============================================================================

/**
 * @brief
 * This handler is called by a hardware timer
 * 1. Check if first frame is loaded
 * 2. Beat time, is the sound beat ISR between each color
 * 3. Fill Ledstick with color
 * 4. If sequence ends, point back to beginig
 * 5. Enable ISR, to wait to next sound beat
 */
void sound_action (void* arg )
{
    
    if(first_beat) // 1
    {
        first_beat = false;
        LedStick->seq_it = LedStick->sequence.begin();
    }
    beat_time = ms_to_us(LedStick->seq_it->time); //2
    
    LedStick->Fill_Led_stick(LedStick->seq_it->group.back().color); //3
    LedStick->seq_it++; // move to next frame

    if(LedStick->seq_it == LedStick->sequence.end()) //4
    {
        first_beat = true;
    }
    sound_released = true;
    input_IO_enable_isr(GPIO_SDD2, GPIO_INTR_NEGEDGE);//5
    hw_timer_deinit();
}
// =============================================================================
// HARDCODED EFFECTS
// =============================================================================

/**
 * @brief
 * This effects show transition between two colors
 * 1. Used to show when IMU is calibrating orange/red
 * 2. IMU succes Green/black
 * 3. Sync in process 
 */
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
/**
 * @brief
 * Alert from some event
 * 1. Calibration abort or error --> FLASH RED
 * 2. System is on AP mode --> FLASH BLUE
 */
void Flash_color(uint8_t R, uint8_t G, uint8_t B, int ms_rate)
{ 
    LedStick->Fill_Led_stick(R,G,B);

    vTaskDelay(ms_rate/ portTICK_RATE_MS);

    LedStick->Led_stick_off();

    vTaskDelay(ms_rate/ portTICK_RATE_MS); 
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


