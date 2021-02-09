#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

//OLED screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//GPIO pin definitions
#define OLED_RESET 4
#define MAX_CS 23
#define MAX_DI 22
#define MAX_DO 21
#define MAX_CLK 20
#define RIGHT_BUTTON 7
#define LEFT_BUTTON 8
#define RIGHT_BUTTON_LED 5
#define LEFT_BUTTON_LED 6
#define HEATING_NO 9
#define PUMP_NO 10 
#define BYPASS_NC 11

//Reference and nominal resistance of PLT100
#define RREF	 430.0
#define RNOMINAL  100.0

#define SETPOINT 92.5
#define MEASUREMENT_INTERVAL 1250
#define PWM_PERIOD (MEASUREMENT_INTERVAL*4.0)

#define K_PROPORTIONAL 0.3
#define K_INTEGRAL 0
#define K_DERIVATIVE 7.5
#define K_NFB 0.4
#define NFB_DECAY 0.7
#define BIAS 0.25

#define MIN_DUTY_CYCLE 0.2
#define MAX_DUTY_CYCLE 0.8

#define ON_BIAS 1.0
#define OFF_BIAS 5.0
#define BIAS_ON_DURATION 5000
#define BIAS_OFF_DURATION 15000

#define DEBOUNCE_TIME 200
#define DEFAULT_BRIGHTNESS 127

#define PRESS 1
#define UNPRESS -1
#define NONE 0


#endif /* DEFINITIONS_H_ */