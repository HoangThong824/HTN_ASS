#include "neo_blinky.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

   while (1) {
    // --- 3 màu lần lượt: Đỏ, Xanh lá, Xanh dương ---
    strip.setPixelColor(0, strip.Color(255, 0, 0));   // Đỏ
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(1000));

    strip.setPixelColor(0, strip.Color(0, 255, 0));   // Xanh lá
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(1000));

    strip.setPixelColor(0, strip.Color(0, 0, 255));   // Xanh dương
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(1000));

    // --- Sau đó tắt dần (fade out) trong 3 giây ---
    const int fadeSteps = 50;         // Số bước mờ dần
    const int fadeDuration = 3000;    // Tổng thời gian 3 giây
    for (int i = fadeSteps; i >= 0; i--) {
        float brightness = (float)i / fadeSteps;
        // Màu trắng mờ dần, có thể đổi màu tùy ý
        strip.setPixelColor(0, strip.Color(
            255 * brightness,
            255 * brightness,
            255 * brightness
        ));
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(fadeDuration / fadeSteps));
    }

    // Tắt hẳn LED
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();

    // Chờ 1 giây trước khi lặp lại chu trình
    vTaskDelay(pdMS_TO_TICKS(1000));
}
}