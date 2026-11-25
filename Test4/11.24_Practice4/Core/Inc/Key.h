#ifndef __KEY_H
#define __KEY_H
#include "main.h"

#define KEY_DEBOUNCE_MS  20

typedef struct
{
    volatile uint8_t tri;
    volatile uint32_t t_last;

    GPIO_TypeDef* port;
    uint16_t pin;

    uint8_t function;
} Key_T;

#endif