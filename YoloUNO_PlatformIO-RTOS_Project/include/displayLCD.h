// lcd
#ifndef __LCD_TASK_H__
#define __LCD_TASK_H__

#include <Arduino.h>
#include "global.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
void lcd_task(void *pvParameters);

#endif