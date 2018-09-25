/* servo motor control example
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

#include "driver/i2c.h"
 
#define LEN 150

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 102 //Maximum angle in degree upto which servo can rotate
#define SERVO_MAX_DEGREE2 120
#define SERVO_MAX_DEGREE3 120

#define BUF_SIZE (8192)

#define ECHO_TEST_TXD  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_RXD  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BLINK_GPIO 12            // PIN A10 - LSB - 12 on the board
#define BLINK_GPIO2 27          // Pin A12 - 27 on the board (right of 12)


// Definitions for Alphanumeric Display
#define DATA_LENGTH                        512              /*!<Data buffer length for test buffer*/
#define RW_TEST_LENGTH                     129              /*!<Data length for r/w test, any value from 0-DATA_LENGTH*/
#define DELAY_TIME_BETWEEN_ITEMS_MS        1234             /*!< delay time between different test items */

#define I2C_EXAMPLE_SLAVE_SCL_IO           22               /*!<gpio number for i2c slave clock  */
#define I2C_EXAMPLE_SLAVE_SDA_IO           23               /*!<gpio number for i2c slave data */
#define I2C_EXAMPLE_SLAVE_NUM              I2C_NUM_0        /*!<I2C port number for slave dev */
#define I2C_EXAMPLE_SLAVE_TX_BUF_LEN       (2*DATA_LENGTH)  /*!<I2C slave tx buffer size */
#define I2C_EXAMPLE_SLAVE_RX_BUF_LEN       (2*DATA_LENGTH)  /*!<I2C slave rx buffer size */

#define I2C_EXAMPLE_MASTER_SCL_IO          22               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          23               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */

#define BH1750_SENSOR_ADDR                 0x70             /*!< slave address for BH1750 sensor */
#define BH1750_CMD_START                   0x23             /*!< Command to set measure mode */
#define ESP_SLAVE_ADDR                     0x70             /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

static esp_err_t i2c_write_oscillator(i2c_port_t i2c_num, uint8_t* data_wr)
{
    // *** This creates a structure (class) called cmd
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // *** adds the i2c start bit into cmd
    i2c_master_start(cmd);
    // *** This adds the alpha i2c driver address to cmd
    i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // *** Add the commmand payload you want to send to device
    i2c_master_write_byte(cmd, 0x21, ACK_CHECK_EN);
    // *** adds the i2c stop bit to cmd
    i2c_master_stop(cmd);
    // *** This command is what puts the cmd payload onto the i2c bus
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t i2c_set_brightness(i2c_port_t i2c_num, uint8_t* data_wr)
{
    // *** This creates a structure (class) called cmd
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // *** adds the i2c start bit into cmd
    i2c_master_start(cmd);
    // *** This adds the alpha i2c driver address to cmd
    i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // *** Add the commmand payload you want to send to device
    i2c_master_write_byte(cmd, *data_wr, ACK_CHECK_EN);
    // *** adds the i2c stop bit to cmd
    i2c_master_stop(cmd);
    // *** This command is what puts the cmd payload onto the i2c bus
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t i2c_set_blink(i2c_port_t i2c_num, uint8_t* data_wr)
{
    // *** This creates a structure (class) called cmd
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // *** adds the i2c start bit into cmd
    i2c_master_start(cmd);
    // *** This adds the alpha i2c driver address to cmd
    i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // *** Add the commmand payload you want to send to device
    i2c_master_write_byte(cmd, *data_wr, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x81, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x3F, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x30, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x38, ACK_CHECK_EN);
    // *** adds the i2c stop bit to cmd
    i2c_master_stop(cmd);
    // *** This command is what puts the cmd payload onto the i2c bus
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t write(i2c_port_t i2c_num, uint16_t * data_wr)
{

    // *** This creates a structure (class) called cmd
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // *** adds the i2c start bit into cmd
    i2c_master_start(cmd);
    // *** This adds the alpha i2c driver address to cmd
    i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // *** Add the commmand payload you want to send to device
      i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[0] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[0] >> 8, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[1] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[1] >> 8, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[2] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[2] >> 8, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[3] & 0xFF, ACK_CHECK_EN);
      i2c_master_write_byte(cmd, data_wr[3] >> 8, ACK_CHECK_EN);

    // i2c_master_write_byte(cmd, 0x81, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x3F, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x30, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
    // i2c_master_write_byte(cmd, 0x38, ACK_CHECK_EN)
    // *** adds the i2c stop bit to cmd
    i2c_master_stop(cmd);
    // *** This command is what puts the cmd payload onto the i2c bus
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static void i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// *** This code won't work until you set the defines correctly !! ***
static void run(int digits[]){
  uint16_t nums[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
  uint8_t osc = 0x21;
  uint8_t* osc_addr = &osc;
  uint8_t brightness = 0xEF;
  uint8_t* brightness_addr = &brightness;
  uint8_t blink = 0x81;
  uint8_t* blink_addr = &blink;
  i2c_write_oscillator(I2C_EXAMPLE_MASTER_NUM, osc_addr);
  i2c_set_brightness(I2C_EXAMPLE_MASTER_NUM, brightness_addr);
  i2c_set_blink(I2C_EXAMPLE_MASTER_NUM, blink_addr);
  uint16_t displaybuffer[] = {nums[digits[0]], nums[digits[1]], nums[digits[2]], nums[digits[3]]};
  uint16_t * displaybuffer_addr = displaybuffer;

  write(I2C_EXAMPLE_MASTER_NUM, displaybuffer_addr);
  // uint8_t data3 = 0x3F;
  // uint8_t* data_addr3 = &data3;
  // i2c_set_blink(I2C_EXAMPLE_MASTER_NUM, data_addr3);
  // uint8_t data4 = 0x3F;
  // uint8_t* data_addr4 = &data4;
  // i2c_set_blink(I2C_EXAMPLE_MASTER_NUM, data_addr4);
  // Now send real commands to the display
  // 1. Send a command to turn on the display oscillator
  // 2. Send a command to set the blink rate or to not blink
  // 3. Send a command to turn the brightness
  // 4. Finally, send a command with bitmap to display a character
  // 5. Have dinner
  // The command values are described on the website or datasheet
}

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
	
    // Initial Values on the Alphanumeric board display 1234
	//int digits[] = {1,2,3,4};
    i2c_example_master_init();
    run(input);

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
    
    struct tm strtime;
    time_t timeoftheday;

    // Initialize the alarm time for the clock
    int alarm_array[4] = {9, 9, 9, 9};

    // Array alphanumeric display is looking at
    int alpha_display[4] = {0, 0, 0, 0};
 
    strtime.tm_year = 2018-1900;
    strtime.tm_mon = 8;
    strtime.tm_mday = 24;
    strtime.tm_hour = input[0] * 10 + input[1];
    strtime.tm_min = input[2] * 10 + input[3];
    strtime.tm_sec = 54;
    strtime.tm_isdst = 0;
    
    timeoftheday = mktime(&strtime);
    
    printf(ctime(&timeoftheday));
	
    uint32_t angle, count, angleminute, anglecount, anglehour, anglecount2, maxdegree2, maxdegree3;
	anglecount = 0;
	anglecount2 = 0; 
	maxdegree2 = (SERVO_MAX_DEGREE2 - (strtime.tm_sec * 2));
	maxdegree3 = (SERVO_MAX_DEGREE3 - strtime.tm_min);
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
				anglecount = 0;
				anglecount2 = 0;
				maxdegree2 = (SERVO_MAX_DEGREE2 - (strtime.tm_sec * 2));
				maxdegree3 = (SERVO_MAX_DEGREE3 - strtime.tm_min);
            }

            // Pressing s will initiate the "Set time" functionality
            // Servos will stop and will change to the new inputed time
            if(dep == 's') {
                int len_set = 0;
                int counter_set = 0;
                while(counter_set != 4) {
                    // Read data from the UART        
                    len_set = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
                    if(len_set != 0) {
                        // Write the data to the buffer
                        uart_write_bytes(UART_NUM_0, (const char *) data, len_set);
                        // Store the correct int into the array
                        input[counter_set] = (int) *data - '0';
                        counter_set = counter_set + 1;
                    }
                }
                strtime.tm_hour = input[0] * 10 + input[1];
                strtime.tm_min = input[2] * 10 + input[3];
                strtime.tm_sec = 0;
				anglecount = 0;
				anglecount2 = 0; 
				maxdegree2 = (SERVO_MAX_DEGREE2 - (strtime.tm_sec * 2));
				maxdegree3 = (SERVO_MAX_DEGREE3 - strtime.tm_min);			
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
        if(strtime.tm_hour == alarm_hour && strtime.tm_min == alarm_minutes && strtime.tm_sec == 0) {
            printf("ALARM HAS BEEN REACHED!\n");
            printf("BEEP! \nBEEP! \nBEEP! \nBEEP! \nBEEP!\n" );
			led_alarm_flash();
        }

		j = 0;
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
				if(anglecount == (maxdegree2) || strtime.tm_sec >= 59)
				{
					anglecount = 0;
					maxdegree2 = SERVO_MAX_DEGREE2;
                    minute_counter = 0;
					anglecount2 = anglecount2 + 1;
					//mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, anglehour);
					if(anglecount2 == (maxdegree3) || strtime.tm_min >= 60)
					{
						anglecount2 = 0;
						maxdegree3 = SERVO_MAX_DEGREE3;
					}
				}
				j = 0;
			}
            vTaskDelay(1.999);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
		}
		strtime.tm_sec = (strtime.tm_sec + 1);
        minute_counter = minute_counter + 1;
		timeoftheday = mktime(&strtime);
	    printf(ctime(&timeoftheday));

        int a = strtime.tm_hour / 10;
        int b = strtime.tm_hour % 10;
        int c = strtime.tm_min / 10;
        int d = strtime.tm_min % 10;
        // alpha_display = {strtime.tm_hour / 10, strtime.tm_hour % 10, strtime.tm_min / 10, strtime.tm_min % 10};
        int lsc[4] = {a, b, c, d};
        run(lsc);
    }
}

void app_main()
{
    printf("Testing servo motor.......\n");
    xTaskCreate(mcpwm_example_servo_control, "mcpwm_example_servo_control", 4096, NULL, 5, NULL);
}
