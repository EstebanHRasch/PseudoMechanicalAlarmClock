/* 
    We used the service control example when building our Quest 1 implementation
    This code allows us to:
        1.) Reset the clock to 00:00:00
        2.) Set the time with the format HH:MM
        3.) Set an alarm with the format HH:MM

    When "make monitor" is executed in the terminal, the user will first need to set the initial time
        of the clock.

    servo motor control example
    This example code is in the Public Domain (or CC0 licensed, at your option.)
    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
 
#define LEN 150

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 102 //Maximum angle in degree upto which servo can rotate
#define SERVO_MAX_DEGREE2 120
#define SERVO_MAX_DEGREE3 60

#define BUF_SIZE (8192)

#define ECHO_TEST_TXD  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_RXD  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BLINK_GPIO 12            // PIN A10 - LSB - 12 on the board
#define BLINK_GPIO2 27          // Pin A12 - 27 on the board (right of 12)

static void mcpwm_example_gpio_initialize()
{
    printf("Initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, 4);    //Set GPIO 18 as PWM0A, to which servo is connected
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 26);
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 25);
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

// Turns the LED on
void switch_one(void) {
    gpio_set_level(BLINK_GPIO, 1); 
}

void switch_two(void) {
    gpio_set_level(BLINK_GPIO2, 1);
}

// Turns all LEDs off
void led_alarm_flash(void) {
    gpio_set_level(BLINK_GPIO, 1); 
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BLINK_GPIO, 0);
    gpio_set_level(BLINK_GPIO2, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BLINK_GPIO2, 0);

    gpio_set_level(BLINK_GPIO, 1); 
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BLINK_GPIO, 0);
    gpio_set_level(BLINK_GPIO2, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BLINK_GPIO2, 0);

    gpio_set_level(BLINK_GPIO, 1); 
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BLINK_GPIO, 0);
    gpio_set_level(BLINK_GPIO2, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BLINK_GPIO2, 0);
}

/**
 * @brief Configure MCPWM module
 */
void mcpwm_example_servo_control(void *arg)
{
    /////////////////////////////
    // Initializing User input //
    /////////////////////////////

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_pad_select_gpio(BLINK_GPIO2);

    // Set the GPIO as a push/pull output 
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);

    printf("Please set the initial time of the clock: \n");
    printf("Note: Enter four digits in the following format - HHMM \n");
    printf("Example - Setting to 2 p.m.: 1400 \n");


    int input[4] = {0, 0, 0, 0};
    int len = 0;
    int counter = 0;
    while(counter != 4) {
        // Read data from the UART        
        len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        if(len != 0) {
            // Write the data to the buffer
            uart_write_bytes(UART_NUM_0, (const char *) data, len);
            // Store the correct int into the array
            input[counter] = (int) *data - '0';
            counter = counter + 1;
        }
    }
    printf("\nYou have set the time to: ");
    // Write data back to the UART
    for(int p = 0; p < 4; p++) {
        printf("%d", input[p]);
    }
    printf("\n");
    // esp_restart();

    struct tm strtime;
    time_t timeoftheday;

    // Initialize the alarm time for the clock
    int alarm_array[4] = {9, 9, 9, 9};
 
    strtime.tm_year = 2018-1900;
    strtime.tm_mon = 8;
    strtime.tm_mday = 24;
    strtime.tm_hour = input[0] * 10 + input[1];
    strtime.tm_min = input[2] * 10 + input[3];
    strtime.tm_sec = 54;
    strtime.tm_isdst = 0;
    
    timeoftheday = mktime(&strtime);
    
    printf(ctime(&timeoftheday));
	
    uint32_t angle, count, angleminute, anglecount, anglehour, anglecount2;
	anglecount = 0;
	anglecount2 = 0; 
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    //printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
	mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);

    int minute_counter = 0;
    int j = 0;

    while (1) {
        len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        if(len != 0) {
            // Write the data to the buffer
            uart_write_bytes(UART_NUM_0, (const char *) data, len);
            // Store the correct int into the array
            char dep = *data;
            printf("You have typed character %c\n", dep);
            // Pressing r will reset the clock to 00:00:00
            if(dep == 'r') {
                strtime.tm_hour = 0;
                strtime.tm_min = 0;
                strtime.tm_sec = 0;
            }

            // Pressing s will initiate the "Set time" functionality
            // Servos will stop and will change to the new inputed time
            if(dep == 's') {
                int input_set[4] = {0, 0, 0, 0};
                int len_set = 0;
                int counter_set = 0;
                while(counter_set != 4) {
                    // Read data from the UART        
                    len_set = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
                    if(len_set != 0) {
                        // Write the data to the buffer
                        uart_write_bytes(UART_NUM_0, (const char *) data, len_set);
                        // Store the correct int into the array
                        input_set[counter_set] = (int) *data - '0';
                        counter_set = counter_set + 1;
                    }
                }
                strtime.tm_hour = input_set[0] * 10 + input_set[1];
                strtime.tm_min = input_set[2] * 10 + input_set[3];
                strtime.tm_sec = 0;
            }

            if(dep == 'a') {
                double start_t, end_t, total_t;

                start_t = clock();
                printf("Starting process: Set Alarm, start_t = %lf\n", start_t);
                    
                int len_alarm = 0;
                int counter_alarm = 0;
                while(counter_alarm != 4) {
                    // Read data from the UART        
                    len_alarm = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
                    if(len_alarm != 0) {
                        // Write the data to the buffer
                        uart_write_bytes(UART_NUM_0, (const char *) data, len_alarm);
                        // Store the correct int into the array
                        alarm_array[counter_alarm] = (int) *data - '0';
                        counter_alarm = counter_alarm + 1;
                    }
                }

                printf("\nAlarm has been set to: ");
                for(int p = 0; p < 4; p++) {
                    printf("%d", alarm_array[p]);
                }
                printf("\n");
                end_t = clock();
                printf("End of process: Alarm Set, end_t = %f\n", end_t);
                
                total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
                printf("Total time of process: %f\n", total_t  );
                printf("Updating servos...\n");

                strtime.tm_hour = strtime.tm_hour + (int) total_t / 3600;
                printf("Hours: %d ", (int) total_t / 3600);

                strtime.tm_min = strtime.tm_min + (int) total_t / 60;
                printf("Minutes: %d ", (int) total_t / 60);

                strtime.tm_sec = strtime.tm_sec + ((int) total_t % 60) + 1;

                // Bug that increments minute by two when one is needed
                minute_counter = minute_counter + ((int) total_t % 60) + 1;
                printf("Seconds: %d ", ((int) total_t % 60) + 1);
                printf("\n");
            }
        }

        // Check if Alarm is the same as the time presently
        int alarm_hour = alarm_array[0] * 10 + alarm_array[1];
        int alarm_minutes = alarm_array[2] * 10 + alarm_array[3];
        if(strtime.tm_hour == alarm_hour && strtime.tm_min == alarm_minutes) {
            printf("ALARM HAS BEEN REACHED!\n");
            printf("BEEP! \nBEEP! \nBEEP! \nBEEP! \nBEEP!\n" );
            led_alarm_flash();
        }

		j = 0;
		//int i = 0;
        for (count = 0; count < SERVO_MAX_DEGREE; count++) {
            //printf("Angle of rotation: %d\n", count);
            angle = servo_per_degree_init(count);
			angleminute = servo_per_degree_init(anglecount);
			anglehour = servo_per_degree_init(anglecount2);
            //printf("pulse width: %dus\n", angle);
            mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
			mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angleminute);
			mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, anglehour);
			
			j++;
			if(j == 45)
			{
				anglecount++;
				//strtime.tm_sec = (strtime.tm_sec + 1);
				//mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angleminute);
				if(anglecount == (SERVO_MAX_DEGREE2 - 1))
				{
					anglecount = 0;
                    if(minute_counter >= 60)
					    strtime.tm_min = (strtime.tm_min + 1);
                    else
                        minute_counter = 0;
					anglecount2 = anglecount2 + 10;
					//mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, anglehour);
					if(anglecount2 == (SERVO_MAX_DEGREE3 - 1))
					{
						anglecount2 = 0;
						strtime.tm_hour = (strtime.tm_hour + 1);
					}
				}
				j = 0;
			}
            vTaskDelay(1.999);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
		}
		strtime.tm_sec = (strtime.tm_sec + 1);
        minute_counter = minute_counter + 1;
		timeoftheday = mktime(&strtime);
        // if(strtime.tm_sec % 2 == 0)
	    printf(ctime(&timeoftheday));
    }
}

void app_main()
{
    printf("Testing servo motor.......\n");
    xTaskCreate(mcpwm_example_servo_control, "mcpwm_example_servo_control", 4096, NULL, 5, NULL);
}