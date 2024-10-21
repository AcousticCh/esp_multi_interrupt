#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* this program with have multiple revisions
   v1 - 2 buttons one led on/off  -- complete
   v2 - 2 buttons 2 leds on/off, LED 2 will be dimmed using pwm
   v3 - 2 buttons 1 led pwm
   v2 - 3 buttons 1 led pwm + on/off */

#define BUTTON_1 26
// check if pin 7 works for input/interrupt
#define BUTTON_2 7

#define LED 27

// replaced by LEDC_OUTPUT_IO
//#define LED_2 25


// led pwm control variables
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO 25 // output pin
#define LEDC_CHANNEL LEDC_CHANNEL_0 // controlled by channel 0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // duty resolution of 13 bits
#define LEDC_DUTY (4096) // 50% duty cycle (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY (4000) // frequency in hertz (4khz)

// start function for led pwm
static void led_pwm(void)
{
	//configure timer
	ledc_timer_config_t ledc_timer = {
		.speed_mode      = LEDC_MODE,
		.timer_num       = LEDC_TIMER,
		.duty_resolution = LEDC_DUTY_RES,
		.freq_hz         = LEDC_FREQUENCY,
		.clk_cfg         = LEDC_AUTO_CLK
	};

	//apply timer
	ledc_timer_config(&ledc_timer);

	//configure channel
	ledc_channel_config_t ledc_channel = {
		.speed_mode      = LEDC_MODE,
		.channel         = LEDC_CHANNEL,
		.timer_sel       = LEDC_TIMER,
		.intr_type       = LEDC_INTR_DISABLE,
		.gpio_num        = LEDC_OUTPUT_IO,
		.duty            = 0,
		.hpoint          = 0
	};

	// apply channel configuration
	ledc_channel_config(&ledc_channel);
}

static void button_trigger_1()
{
	gpio_set_level(LED, 1);
	//gpio_set_level(LED_2, 1);

	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}


static void button_trigger_2()
{
        gpio_set_level(LED, 0);
	//gpio_set_level(LED_2, 0);

	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}


void app_main(void)
{
	// setup pin direction, pull, interrrupt type and level

	//calling ledc_function
	led_pwm();

	// BUTTONS
	gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
        gpio_set_direction(BUTTON_2, GPIO_MODE_INPUT);

	//button 1 uses a pullup resistor keeping the pin at Vcc until button is pressed switching the V from vcc to 0v for a negative/falling edge
	gpio_set_pull_mode(BUTTON_1, GPIO_PULLUP_ONLY);

	// button 2 uses a pulldown resistor keeping the pin at 0v until the button is pressed allowing vcc to flow through pin causing a positive/rising edge
        gpio_set_pull_mode(BUTTON_2, GPIO_PULLDOWN_ONLY);

	gpio_set_intr_type(BUTTON_1, GPIO_INTR_NEGEDGE);
        gpio_set_intr_type(BUTTON_2, GPIO_INTR_POSEDGE);


	// LEDS
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);

	//gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
	//setup interrupt handler service

	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON_1, button_trigger_1, NULL);
        gpio_isr_handler_add(BUTTON_2, button_trigger_2, NULL);

	gpio_intr_enable(BUTTON_1);
	gpio_intr_enable(BUTTON_2);


	while(1)
	{
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
