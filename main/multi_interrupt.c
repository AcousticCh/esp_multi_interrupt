#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
this program with have multiple revisions
   v1 - 2 buttons one led on/off  -- complete
   v2 - 2 buttons 2 leds on/off, LED 2 will be dimmed using pwm -- complete
   v3 - 2 buttons 1 led pwm -- complete
   v4 - 3 buttons 1 led pwm + on/off -- complete. might be refactored
*/

#define BUTTON_ON_OFF 5

#define BUTTON_1 26

#define BUTTON_2 7

#define LED 25

// led pwm control variables
#define TIMER_NUM LEDC_TIMER_0
#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define LED_PIN 27 // output pin
#define CHANNEL LEDC_CHANNEL_0 // controlled by channel 0
#define DUTY_CYCLE_BITS LEDC_TIMER_13_BIT // duty resolution of 13 bits
#define DUTY_CYCLE (0) // 50% duty cycle (2 ** 13) * 50% = 4096
#define FREQ_HZ (4000) // frequency in hertz (4khz)
#define FADE_TIME_MS 1000 // 1 second fade time, value in milliseconds

/*
button state 0 is doing nothing
state 1 is rising duty cycle
state 2 is falling duty cycle
*/

uint32_t button_on_off = 0; // checks if button on off was pressed
uint32_t button_state = 0; // changed by button 1 and button 2 interrupt handlers for condition checking
uint32_t duty_steps[] = {0, 1000, 2000, 3000, 4000}; // array of duty cycles chosen with duty selector
uint32_t duty_selector = 0; // this variables integer  is used to get and change current duty cycle


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


// button on off handler
static void button_on_off_h()
{

	if(button_on_off == 0)
	{
		button_on_off = 1;
	} else if(button_on_off == 1)
	{
		button_on_off = 0;
	};
}


// button 1 handler
static void button_trigger_1()
{
	button_state = 1;
}


// button 2 handler
static void button_trigger_2()
{
	button_state = 2;
}


void app_main(void)
{
	// setup pin direction, pull, interrrupt type and level

	//calling ledc_function
	led_pwm();

	// BUTTONS
	gpio_set_direction(BUTTON_ON_OFF, GPIO_MODE_INPUT);

	gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
        gpio_set_direction(BUTTON_2, GPIO_MODE_INPUT);


	gpio_set_pull_mode(BUTTON_ON_OFF, GPIO_PULLUP_ONLY);

	//button 1 uses a pullup resistor keeping the pin at Vcc until button is pressed switching the V from vcc to 0v for a negative/falling edge
	gpio_set_pull_mode(BUTTON_1, GPIO_PULLUP_ONLY);

	// button 2 uses a pulldown resistor keeping the pin at 0v until the button is pressed allowing vcc to flow through pin causing a positive/rising edge
        gpio_set_pull_mode(BUTTON_2, GPIO_PULLDOWN_ONLY);




	// interrupt type
	gpio_set_intr_type(BUTTON_ON_OFF, GPIO_INTR_NEGEDGE);
	gpio_set_intr_type(BUTTON_1, GPIO_INTR_NEGEDGE);
        gpio_set_intr_type(BUTTON_2, GPIO_INTR_POSEDGE);


	// LEDS
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);

	//setup interrupt handler service
	gpio_install_isr_service(0);

	// connect interrupt to interrupt handler
	gpio_isr_handler_add(BUTTON_ON_OFF, button_on_off_h, NULL);
	gpio_isr_handler_add(BUTTON_1, button_trigger_1, NULL);
        gpio_isr_handler_add(BUTTON_2, button_trigger_2, NULL);

	gpio_intr_enable(BUTTON_ON_OFF);
	gpio_intr_enable(BUTTON_1);
	gpio_intr_enable(BUTTON_2);

	//install ledc fading capability
	ledc_fade_func_install(0);


	while(1)
	{

		if(button_on_off == 1)
		{
			if(duty_selector > 0)
			{
			ledc_set_fade_with_time(SPEED_MODE, CHANNEL, duty_steps[duty_selector], 50);
			ledc_fade_start(SPEED_MODE, CHANNEL, LEDC_FADE_WAIT_DONE);
			};

			switch(button_state)
			{
				case 1:
					// reset button state
                	        	button_state = 0;

					// CREATE A FUMCTION TO INCREMENT SELECTOR

	                        	// raise duty cycle
					if(duty_selector < 4)
                                        {
                                                ++duty_selector;
                                        };

	                        	// fade to updated duty cycle
	                        	ledc_set_fade_with_time(SPEED_MODE, CHANNEL, duty_steps[duty_selector], FADE_TIME_MS);
	                        	ledc_fade_start(SPEED_MODE, CHANNEL, LEDC_FADE_WAIT_DONE);
					break;

				case 2:
					// reset button state
	                        	button_state = 0;

	                        	// lower duty cycle
					if(duty_selector > 0)
					{
						--duty_selector;
					};

	                        	// fade to updated duty cycle
	                        	ledc_set_fade_with_time(SPEED_MODE, CHANNEL, duty_steps[duty_selector], FADE_TIME_MS);
	                        	ledc_fade_start(SPEED_MODE, CHANNEL, LEDC_FADE_WAIT_DONE);
					break;

			}; //end of switch statement
		} else
		{
			ledc_set_fade_with_time(SPEED_MODE, CHANNEL, 0, 50);
			ledc_fade_start(SPEED_MODE, CHANNEL, LEDC_FADE_WAIT_DONE);
		}; //end of if statement


		// loop delay of 100 ms
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}; // end of while statement
} // end of main
