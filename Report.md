# EC444 Quest 1

## Quest Name: Pseudo-Mechanical Clock

### Author: David Li, Gordon Wallace, Esteban Hernandez

### Date: 2018-09-25

### Summary: 

**Quest 1: Pseudo-Mechanical Alarm Clock Report**


**Assignment**

The goal of this quest was to build an alarm clock that incorporates an alphanumeric display that shows the time in hours and minutes, two servos to display the time like a traditional analog clock, and LED lights that flash when the alarm time is reached.  The user had to be able to set the clock time and alarm time as well.  Our final product succeeded at all of these objectives.

**Implementation**

There are three main parts to the C code structure: the user input, the alphanumeric display, and the servo control. Each of these parts were written using a base esp-idf example for reference.  When each member was able to successfully implement his assigned part, we then combined all the individual components into one C file.

The user input is based on the code found in the esp directory ..esp-idf/examples/peripherals/uart_echo. The skill assignment Use GPIO to Drive LEDs was first completed to understand how uart enables communication between two devices. The buffer size must be set large enough or the pipe holding the data will not have enough space to hold the inputted digits. Regarding the initialization of user input, echo test parameters were disabled, and uart was configured at a baud rate of 115200, data bit of 8, and disabled uart parity. When “make monitor” is executed after the esp board has been flashed, the user will need to set the initial time for the clock. All time inputs to the terminal is in military time format, HHMM, with no spaces in between. The user can enter the digits on their computer’s keyboard, and the enter key does not need to be pressed as the program recognizes the input automatically after each digit is typed. Once the initial time for the clock has been set, the user can at any point press ‘r’, ‘s’, or ‘a’ to reset the clock to 00:00, set a new time for the clock, or set the alarm time respectively. The alarm signal is represented as two LEDs on the breadboard. When the current time of the clock reaches the set time on the alarm, the LEDs will flash for 1.5 seconds and a notification will appear on the terminal where ‘make monitor’ was executed. The user input only requires the esp board and the user’s computer to operate. However, the user input drives the numbers displayed on the alphanumeric display and the angles on the servo.

In order to display the time the user sets on the alphanumeric display, our program uses the I2C protocol to communicate with the matrix.  Built in functions from the matrix driver are used to configure the ESP32 board in master mode and then write to the display.  Initially, configuration settings are specified, such as the SDA and SCL ports and the clock speed.  Pull-up resistors are also enabled during this initialization process.  To prepare the alphanumeric display, I2C instructions to start the oscillator, set the brightness, and set the blink rate are sent to the display through a command object.  These are followed by instructions containing the binary codes which dictate which segments of the display will light up to produce the desired characters.  When a command is written to the display, it begins with an instruction specifying the slave address and that a write operation will be taking place.  As the time increments or when the user enters a new time, the display will update by following this procedure.  The display code is largely based on the code provided in ..esp-idf/examples/peripherals/i2c and the example provided on the course repository.  To communicate with the display, the SDA and SCL ports of the ESP32 are wired to the corresponding ports on the alphanumeric display, as well as the MO and MI ports on the ESP32 in order to make use of the pull-up resistors.  The display is powered from the 3V port of the ESP32, and is grounded at GND.

The servo component of our project is the backbone of the clock, because they are what makes the clock tick.  Through trial and error we were able to get the right parameters to get the servos running exactly one tick per second. From there we could get it to exactly one minute and one hour.  Our servos run in a 120 degree clock, which means that for every second, the angle of the minute servos increases by 2 degrees until it reaches 120 degrees.  Once the servo reaches 120 degrees it then increments the hour servo by 2 degrees until the hour servo reaches 120 degrees representing that a full hour has passed.  It will also increment the time struct which is keeping track of our local/set time after which it will reset back to 0 degrees, its starting position, and start all over again.  When our clock first starts up, the user has to input the time.  Once they input the time, the servos will read the amount of minutes there are (using the time struct)  and adjust the angle of the hour servo according to the time set. The program does the same with the minute servos by reading the time struct of seconds.  After the time is set, the servos will cycle with the time struct representing the actual time in a 120 degree clock.

**Results and Conclusion**

Our group was able to successfully implement the required objectives from Quest 1. The user can input digits to be read by the program, displayed onto the servos, and flashed to the alphanumeric display. The clock can be reset and set to a new time at any moment when the clock is operational and keeps time precisely. An alarm can be set, and will trigger when the time has been reached by flashing two separate LEDs.  The ESP32 successfully starts the alphanumeric display and writes digits to it using the I2C protocol. The servos component is what drives the clock.  By incrementing the time struct, we built every second. The program creates an internal clock which we used to tell what angle the hours/minutes servos should be at representing the actual/set time. This cycle repeats until the program is terminated.

**Summary**

**Evaluation Criteria**
We decided on the following specifications for a successful solution to this quest:

* Two servos indicate current time in minutes and seconds
* User can set an alarm which triggers when current time reaches the set alarm time
* Alphanumeric Display shows current time
* Components stay updated under changes made by the user

**Solution Design**

We used a Adafruit HUZZAH32 - ESP32 Feather board, two J-Deal SG90 Micro Servo Motors, and a Adafruit 0.56" 4-Digit 14-Segment Display w/ FeatherWing Combo Pack. The pins used for the clock are:

* 
* 
* 
* 
* 
* 
* 
* 

### Sketches and Photos:

<div class="row">
  <div class="column">
    <img src= "https://github.com/EstebanHernandez1/EC444_Quest1/blob/master/first_alpha_build.JPG" width="40%" alt="First alphanumeric build"> 
  </div>
  <div class="column">
    <img src= "https://github.com/EstebanHernandez1/EC444_Quest1/blob/master/missing_alpha.JPG" width="40%" alt="Missing Alphanumeric Display"> 
  </div>
</div>
<div class="row">
  <div class="column">
    <img src= "https://github.com/EstebanHernandez1/EC444_Quest1/blob/master/v0_esp_alpha.JPG" width="40%" alt="Final Version: Alphanumeric"> 
  </div>
  <div class="column">
    <img src= "https://github.com/EstebanHernandez1/EC444_Quest1/blob/master/v0_esp_leds.JPG" width="40%" alt="Final Version: LEDs"> 
  </div>
</div>
<div class="row">
  <div class="column">
    <img src= "https://github.com/EstebanHernandez1/EC444_Quest1/blob/master/v0_esp_servo.JPG" width="40%" alt="Final Version: Servos"> 
  </div>
  <div class="column">
    <img src= "https://github.com/EstebanHernandez1/EC444_Quest1/blob/master/initial_plan.JPG" width="40%" alt="Initial Plan"> 
  </div>
</div>


### Modules, Tools, Source Used in Solution
(1) https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/using-with-arduino-ide <br/>
(2) https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html <br/>
(3) https://github.com/espressif/arduino-esp32/issues/333 <br/>
(4) https://cdn-learn.adafruit.com/downloads/pdf/adafruit-huzzah32-esp32-feather.pdf <br/>
(5) https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/pinouts <br/>
(6) http://whizzer.bu.edu/individual-assignments/3-sensor-actuator/alpha-display <br/>
(7) http://whizzer.bu.edu/individual-assignments/3-sensor-actuator/servo <br/>
(8) http://whizzer.bu.edu/team-quests/primary/pseudo-clock <br/>
(9) http://whizzer.bu.edu/individual-assignments/2-single-micro/gpio-drive-leds <br/>
(10) https://en.wikipedia.org/wiki/C_date_and_time_functions <br/>
(11) https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet <br/>
(12) https://learn.adafruit.com/14-segment-alpha-numeric-led-featherwing/pinouts <br/>

### Supporting Artifacts:
The repository containing all of the code and documentation for this project can be found at https://github.com/EstebanHernandez1/EC444_Quest1.

The video recorded can be found here at 

