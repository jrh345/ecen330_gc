#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "watch.h"
#include "hw_gc.h"

#define ISR_MAX_CNT 500
static const char *TAG = "lab03";

volatile bool running;
volatile uint64_t timer_ticks;

int64_t start, finish;

volatile int64_t isr_max = 0; // Maximum ISR execution time (us)
volatile int32_t isr_cnt = 0; // Count of ISR invocations

gptimer_handle_t gptimer = NULL;
gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
};

static bool example_timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)

{
    start = esp_timer_get_time();

    if (!pin_get_level(HW_BTN_A)) {
    running = true;
    } 
    else if (!pin_get_level(HW_BTN_B)) 
    {
        running = false;
    } 
    else if (!pin_get_level(HW_BTN_START)) 
    {
        running = false;
        timer_ticks = 0;
    } 
    
    if (running)
    {
        timer_ticks++;
    }

    finish = esp_timer_get_time();
    int64_t elapsed = finish - start;

    isr_max = (elapsed > isr_max) ? elapsed : isr_max;
    isr_cnt++;

    return false;

}

gptimer_alarm_config_t alarm_config = {

    .reload_count = 0,      // When the alarm event occurs, the timer will automatically reload to 0

    .alarm_count = 10000, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s

    .flags.auto_reload_on_alarm = true, // Enable auto-reload function

};

gptimer_event_callbacks_t cbs = {

    .on_alarm = example_timer_on_alarm_cb, // Call the user callback function when the alarm event occurs

};




// Main application
void app_main(void)
{
    

    ESP_LOGI(TAG, "Starting");

    start = esp_timer_get_time();
    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_B);
    pin_reset(HW_BTN_START);

    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    pin_input(HW_BTN_START, true);
    finish = esp_timer_get_time();
    printf("Pin Config Section time:%lld microseconds\n", finish-start);
	
    

    
    start = esp_timer_get_time();
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Set the timer's alarm action
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // Register timer event callback functions, allowing user context to be carried
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    finish = esp_timer_get_time();
    printf("Stopwatch Config Section time:%lld microseconds\n", finish-start);


    start = esp_timer_get_time();

    ESP_LOGI(TAG, "Stopwatch update");

    finish = esp_timer_get_time();
    printf("ESP_LOGI Section time:%lld microseconds\n", finish-start);

    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    for (;;) { // forever update loop
        watch_update(timer_ticks);
        if (isr_cnt >= ISR_MAX_CNT) 
        {
            printf("Maximum ISR execution time: %lld microseconds\n", isr_max);
            isr_max = 0;
            isr_cnt = 0;
        }
        
    }
}

