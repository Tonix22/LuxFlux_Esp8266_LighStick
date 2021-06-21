#ifndef LIGHT_EFFECTS_H
#define LIGHT_EFFECTS_H

#ifdef __cplusplus
extern "C" 
{
#endif

    // =============================================================================
    // C Includes
    // =============================================================================
    #include "FreeRTOS_wrapper.h"
    // =============================================================================
    // Defines
    // =============================================================================

    #define ms_to_us(delay) delay*1000
    // =============================================================================
    // Type definitions
    // =============================================================================
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
        CIRC_LOAD,
        TCP_LOAD,
        TCP_SYNC
    }Light_MessageID;

    typedef enum
    {
        RAINBOW,
        END
    }test_Vane;
    // =============================================================================
    // C Visible Function prototypes
    // =============================================================================

    void Light_task(void *arg);
    void Ligth_init(void);
    void Pixel_rainbow(void);
    void Flash_color(uint8_t R, uint8_t G, uint8_t B, int ms_rate);
    void Fade_color(void);
    void Fade_colorG(void);
    void Pixel_rainbow_Fade(void);
    void Pixels_OFF();
    EventBits_t IDLE_light();
    void Set_Frames_buffer(uint8_t frames);

    #ifdef __cplusplus
        // =============================================================================
        // C++ Includes
        // =============================================================================
        #include "neopixel.h"
        #include "IO_driver.h"
        #include "structs.h"
        #include "Menu.h"
        #include "SimpleIMU_6.h"
        #include <vector>
        #include <list>
        // =============================================================================
        // Type definitions
        // =============================================================================
        typedef std::list<RGB>* list_ptr;

        /*
        Reads from flash , and write data into a linked list.
        +-----+-------+------+
        | Red | Green | Blue | --> RGB
        +-----+-------+------+

        +--------+------+
        | Pixels | RGB  | --> BLOCK
        +--------+------+

        +-------+-------+----+-------------+
        | time  | group | .. | list<Block> | --> Frame
        +-------+-------+----+-------------+

        +--------+---------+-----+--------------+
        | Frame1 | Frame2  | ... | list<Frames> | --> Sequence
        +--------+---------+-----+--------------+

        */

       /* =============================================================================
             ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
            ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
            ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
            ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
            ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
            ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
        =============================================================================*/

        class Light
        {
            
            public:
            // a list of frames is a sequence
            std::list<Frame> sequence; // Light sequence list buffer
            std::list<Frame>::iterator seq_it;// Helper iterator to pass over the list

            uint8_t pixels        ; // modular pixes in neopixel
            uint8_t max_frames    ; // max buffer size
            uint8_t rainbow_delay ; // test rainbow
            

            Light(uint8_t numer_of_pixels)
            {
                this->pixels = numer_of_pixels;
                this->max_frames = 10; // max frames in RAM
                this->rainbow_delay = 10;
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