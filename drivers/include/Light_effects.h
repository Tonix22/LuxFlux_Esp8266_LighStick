#ifndef LIGHT_EFFECTS_H
#define LIGHT_EFFECTS_H

typedef enum 
{
    RED,
    GREEN,
    BLUE,
}Color;

void Pixel_rainbow(void);
void Flash_color(uint8_t R, uint8_t G, uint8_t B, int ms_rate);
void Fade_color(void);
#endif