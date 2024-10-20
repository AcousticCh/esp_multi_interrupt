#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* this program with have multiple revisions
   v1 - 2 buttons one led on/off
   v2 - 2 buttons 2 leds on/off
   v3 - 2 buttons 1 led pwm
   v2 - 3 buttons 1 led pwm + on/off */

#define BUTTON_1 26
// check if pin 7 works for input/interrupt
#define BUTTON_2 7

#define LED 27


static void button_trigger_1()
{
	gpio_set_level(LED, 1);
}


static void button_trigger_2()
{
        gpio_set_level(LED, 0);
}


void app_main(void)
{
	// setup pin direction, pull, interrrupt type and level

	// BUTTONS
	gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
        gpio_set_direction(BUTTON_2, GPIO_MODE_INPUT);

	//button 1 uses a pullup resistor keeping the pin at Vcc until button is pressed switching the V from vcc to 0v for a negative/falling edge
	gpio_set_pull_mode(BUTTON_1, GPIO_PULLUP_ONLY);

	// button 2 uses a pulldown resistor keeping the pin at 0v until the button is pressed allowing vcc to flow through pin causing a positive/rising edge
        gpio_set_pull_mode(BUTTON_2, GPIO_PULLDOWN_ONLY);

	gpio_set_intr_type(BUTTON_1, GPIO_INTR_NEGEDGE);
        gpio_set_intr_type(BUTTON_2, GPIO_INTR_POSEDGE);


	// LED
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);

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
