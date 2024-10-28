#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* this program with have multiple revisions
   v1 - 2 buttons one led on/off  -- complete
   v2 - 2 buttons 2 leds on/off, LED 2 will be dimmed using pwm -- complete
   v3 - 2 buttons 1 led pwm -- in progress
   v2 - 3 buttons 1 led pwm + on/off */

#define BUTTON_1 26
// check if pin 7 works for input/interrupt
#define BUTTON_2 7

#define LED 25

// replaced by LEDC_OUTPUT_IO
//#define LED_2 25


// led pwm control variables
#define TIMER_NUM LEDC_TIMER_0
#define SPEED_MODE LEDC_LOW_SPEED_MODE
#define LED_PIN 27 // output pin
#define CHANNEL LEDC_CHANNEL_0 // controlled by channel 0
#define DUTY_CYCLE_BITS LEDC_TIMER_13_BIT // duty resolution of 13 bits
#define DUTY_CYCLE (0) // 50% duty cycle (2 ** 13) * 50% = 4096
#define FREQ_HZ (4000) // frequency in hertz (4khz)
#define MAX_DUTY_CYCLE (4096)

#define FADE_TIME_MS 1000

/*
button state 0 is doing nothing
state 1 is rising duty cycle
state 2 is falling duty cycle
*/

uint32_t button_state = 0;
uint32_t current_duty_cycle = 0;


// start function for led pwm
static void led_pwm()
{
	//configure timer
	ledc_timer_config_t ledc_timer = {
		.speed_mode      = SPEED_MODE,
		.timer_num       = TIMER_NUM,
		.duty_resolution = DUTY_CYCLE_BITS,
		.freq_hz         = FREQ_HZ,
		.clk_cfg         = LEDC_AUTO_CLK
	};

	//apply timer
	ledc_timer_config(&ledc_timer);

	//configure channel
	ledc_channel_config_t ledc_channel = {
		.speed_mode      = SPEED_MODE,
		.channel         = CHANNEL,
		.timer_sel       = TIMER_NUM,
		.intr_type       = LEDC_INTR_DISABLE,
		.gpio_num        = LED_PIN,
		.duty            = DUTY_CYCLE,
		.hpoint          = 0
	};

	// apply channel configuration
	ledc_channel_config(&ledc_channel);
}

static void button_trigger_1()
{
	button_state = 1;

	//gpio_set_level(LED, 1);
	//gpio_set_level(LED_2, 1);

	//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
	//ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}


static void button_trigger_2()
{
	button_state = 2;

        //gpio_set_level(LED, 0);
	//gpio_set_level(LED_2, 0);

	//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        //ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
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

	//install ledc fading capability
	ledc_fade_func_install(0);


	while(1)
	{

		if(button_state == 1)
		{
			button_state = 0;

			//ledc_get_duty(SPEED_MODE, CHANNEL);
			//raise duty cycle
			current_duty_cycle = current_duty_cycle + 1000;

			ledc_set_fade_with_time(SPEED_MODE, CHANNEL, current_duty_cycle, FADE_TIME_MS);
			ledc_fade_start(SPEED_MODE, CHANNEL, LEDC_FADE_WAIT_DONE);
		} else if(button_state == 2)
		{
			button_state = 0;
			//lower duty cycle
			current_duty_cycle = current_duty_cycle - 1000;

			ledc_set_fade_with_time(SPEED_MODE, CHANNEL, current_duty_cycle, FADE_TIME_MS);
                        ledc_fade_start(SPEED_MODE, CHANNEL, LEDC_FADE_WAIT_DONE);
		};

		vTaskDelay(100 / portTICK_PERIOD_MS);
	};
}
