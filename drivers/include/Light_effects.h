#ifndef LIGHT_EFFECTS_H
#define LIGHT_EFFECTS_H

#ifdef __cplusplus
extern "C" 
{
#endif
    #include "FreeRTOS_wrapper.h"
    typedef enum
    {
        ENABLE_SOUND,
        SOUND,
        IMU,
        FADE,
        WIFI_CMD,
        CALIBRATION,
        END_CALIBRATION,
        ABORT_CALIBRATION,
        OFF,
        LOAD_EFFECTS,
    }Light_MessageID;

    void Light_task(void *arg);
    void Ligth_init(void);
    void Pixel_rainbow(void);
    void Flash_color(uint8_t R, uint8_t G, uint8_t B, int ms_rate);
    void Fade_color(void);
    void Fade_colorG(void);
    void Pixel_rainbow_Fade(void);
    EventBits_t IDLE_light();
    void Set_Frames_buffer(uint8_t frames);
    #ifdef __cplusplus
        #include "neopixel.h"
        #include "IO_driver.h"
        #include "structs.h"
        #include "Menu.h"
        #include <vector>
        #include <list>
        typedef std::list<RGB>* list_ptr;
        class Light
        {
            
            public:
            // a list of frames is a sequence
            std::list<Frame> sequence;
            std::list<Frame>::iterator seq_it;
            uint8_t pixels        ;
            uint8_t max_frames    ;
            uint8_t rainbow_delay ;
            int flash_times       ;
            int fade_cycles       ;
            

            
            Light(uint8_t numer_of_pixels)
            {
                this->pixels = numer_of_pixels;
                this->max_frames = 10; // max frames in RAM
                this->rainbow_delay = 10;
                this->flash_times = 1000;
                this->fade_cycles = 1000;
            }
            

            std::vector<RGB> Fade_colors
            {
                {255, 0  , 0},
                {255, 128, 0},
            };

            std::vector<RGB> Pixel_Rainbow
            {
                {255, 0,   0  },
                {255, 165, 0  },
                {255, 255, 0  },
                {0,   128, 0  },
                {28,  188, 196},
                {0,   0,   255},
                {75,  0,   130},
                {238, 130, 238},
            };

            void Fill_Led_stick(uint8_t R,uint8_t G,uint8_t B)
            {
                for(int led = 0; led < this->pixels ;led++)
                {
                    Color_Frame(&R,&G,&B);
                }
            }
            void Fill_Led_stick(RGB& LED)
            {
                for(int led = 0; led < this->pixels ;led++)
                {
                    Color_Frame(&LED.RED, &LED.GREEN,&LED.BLUE);
                }
            }
            inline void Paint_LED(RGB& LED)
            {
                Color_Frame(&LED.RED, &LED.GREEN,&LED.BLUE);
            }
            inline void Led_stick_off(void)
            {
                Fill_Led_stick(0,0,0);
            }
        };
}
    #endif

#endif