// lcd_task.cpp
#include "displayLCD.h"


// Thay 0x27 hoặc 0x3F tùy màn hình của bạn
LiquidCrystal_I2C lcd(0x27, 16, 2);  // 16x2 LCD
// LiquidCrystal_I2C lcd(0x27, 20, 4); // nếu dùng 20x4

void lcd_task(void *pvParameters) {
  Wire.begin();
  lcd.init();
  lcd.backlight();

  // Tạo ký tự đặc biệt
  uint8_t degC[8]   = {0x08,0x14,0x08,0x03,0x04,0x04,0x04,0x03};
  uint8_t humidity[8] = {0x04,0x04,0x0A,0x0A,0x11,0x11,0x11,0x0E};
  uint8_t sun[8]     = {0x00,0x0A,0x15,0x1F,0x1F,0x15,0x0A,0x00};

  lcd.createChar(1, degC);
  lcd.createChar(2, humidity);
  lcd.createChar(3, sun);

  lcd.clear();
  lcd.print("  ESP32 IoT v2  ");
  lcd.setCursor(0,1);
  lcd.print("  Dang khoi dong ");
  delay(2000);

  for (;;) {
    xSemaphoreTake(lcdMutex, portMAX_DELAY);  // Bảo vệ LCD

    lcd.clear();

    // Dòng 1: Nhiệt độ + Độ ẩm
    lcd.setCursor(0,0);
    lcd.print("T:");
    lcd.print(glob_temperature, 1);
    lcd.write(1);       // °C
    lcd.print(" ");

    lcd.setCursor(9,0);
    lcd.print("H:");
    lcd.print((int)glob_humidity);
    lcd.write(2);       // biểu tượng %
    lcd.print("%");

    // Dòng 2: Ánh sáng
    lcd.setCursor(0,1);
    lcd.write(3);       // mặt trời
    lcd.print(":");
    lcd.print((int)glob_light);
    lcd.print("lx ");

    // Cảnh báo trời tối (dưới 100 lx)
    if (glob_light < 100) {
      static bool blinkState = false;
      blinkState = !blinkState;
      if (blinkState) lcd.backlight();
      else lcd.noBacklight();
    } else {
      lcd.backlight();
    }

    xSemaphoreGive(lcdMutex);

    vTaskDelay(pdMS_TO_TICKS(2500));  // Cập nhật 2.5 giây/lần
  }
}