/* ============================================================
 * Servo.c  —  SG90（180° 型）舵机驱动
 * 芯片：STM32F103C8T6      库：HAL
 * ============================================================ */
#include "Servo.h"
#include "OLED.h"

/* CubeMX 生成的定时器句柄，名字要和 main.c 里的一致 */
extern TIM_HandleTypeDef htim2;

/**
 * @brief  角度 -> CCR
 * @param  angle  0~180
 * @retval 写进比较寄存器的值
 *
 * 前提：PSC 和 ARR 已经配成"1 个计数 = 1us"，
 *       所以 CCR 的数值就等于脉宽的微秒数。
 */
uint16_t Servo_AngleToCCR(float angle)
{
    if (angle < SERVO_MIN_DEG)   angle = SERVO_MIN_DEG;
    if (angle > SERVO_MAX_DEG)   angle = SERVO_MAX_DEG;

    return (uint16_t)(SERVO_MIN_US + (angle - SERVO_MIN_DEG) / (SERVO_MAX_DEG - SERVO_MIN_DEG) * (SERVO_MAX_US - SERVO_MIN_US));
}

/**
 * @brief  设置舵机角度
 */
uint16_t Servo_SetAngle(float angle)
{
    uint16_t ccr = Servo_AngleToCCR(angle);
    __HAL_TIM_SET_COMPARE(SERVO1_TIM_HANDLE, SERVO1_CHANNEL, ccr);
    return ccr;
}

/**
 * @brief  在 OLED 上显示三行
 *
 * 目标格式（和文档 9.6 节一致，方便验收）：
 *   第 1 行   Target:090deg
 *   第 2 行   CCR   :1500
 *   第 3 行   Pulse :1.500ms
 */
void Servo_ShowOnOLED(float angle, uint16_t ccr)
{
    char oled_buffer[80] = {0};

    snprintf(oled_buffer, sizeof(oled_buffer), "Target:%03ddeg", (int)angle);
    OLED_ShowString(1,1,oled_buffer);
    snprintf(oled_buffer, sizeof(oled_buffer), "CCR   :%4d"    , (int)ccr);
    OLED_ShowString(2,1,oled_buffer);
    snprintf(oled_buffer, sizeof(oled_buffer), "Pulse :%d.%03dms", ccr / 1000, ccr % 1000);
    OLED_ShowString(3, 1, oled_buffer);
}
