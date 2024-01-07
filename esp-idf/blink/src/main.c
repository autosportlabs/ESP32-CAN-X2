#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2

static const char *TAG = "ESP32-CAN-X2";

void led_callback() {
    static bool state = true;

    state = state != true;

    gpio_set_level(LED_PIN, state);
}

void app_main() {
    ESP_LOGI(TAG, "Start");

    gpio_config_t led_config;
    led_config.pin_bit_mask = 0x01L << LED_PIN;
    led_config.intr_type = GPIO_INTR_DISABLE;
    led_config.mode = GPIO_MODE_OUTPUT;
    led_config.pull_up_en = GPIO_PULLUP_ENABLE;
    led_config.pull_down_en = GPIO_PULLDOWN_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&led_config));


    esp_timer_handle_t led_timer_handle;
    esp_timer_create_args_t timer_config;
    timer_config.dispatch_method = ESP_TIMER_TASK;
    timer_config.name = "LED Timer";
    timer_config.callback = led_callback;
    ESP_ERROR_CHECK(esp_timer_create(&timer_config, &led_timer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(led_timer_handle, 1000000));
}