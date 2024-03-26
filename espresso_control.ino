  #include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Adafruit_MAX31865.h>
#include "definitions.h"

//#define VERBOSE

class Button{
  private:
    int pin;
    int state;
    int last_state;
    unsigned long debounce_time;
    unsigned long last_event;
  public:
    Button(int pin, unsigned long debounce_time){
      this->pin = pin;
      this->debounce_time = debounce_time;
      this->last_event = 0;
      this->state = HIGH;
      this->last_state = HIGH;

      pinMode(pin, INPUT_PULLUP);
    }
    int poll(){
      
      int reading = digitalRead(pin);
      if (reading != last_state){
        last_event = millis();
      }

      last_state = reading;

      if (millis() - last_event >= debounce_time){
        if (reading != state){
          state = reading;
          if (state == LOW){
            return PRESS;
          }
          if (state == HIGH){
            return UNPRESS;
          }
        }
      }
    return NONE;
    }
    
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 max = Adafruit_MAX31865(MAX_CS, MAX_DI, MAX_DO, MAX_CLK);

float temperature;
float duty_cycle = 0.0;

float measurement_array[4];
int measurement_index = 0;

float error_integral = 0.0;
float feedback = 0.0;

unsigned long time_ms;
unsigned long last_reading_time = 0;
unsigned long pwm_start_time;

unsigned long bias_on_time = 0;
unsigned long bias_off_time = 0;

int pwm_active = 0;
int brew_active = 0;

int bias_on = 0;
int bias_off = 0;

Button left_button = Button(LEFT_BUTTON, 50);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  max.begin(MAX31865_3WIRE);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&FreeSans12pt7b);
  display.clearDisplay();
  display.display();
  
  delay(2000);
  Serial.print("Time, Temperature \n");
  
  pinMode(LEFT_BUTTON_LED, OUTPUT);
  pinMode(RIGHT_BUTTON_LED, OUTPUT);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(BYPASS_NC, OUTPUT);
  digitalWrite(BYPASS_NC, HIGH);
  pinMode(HEATING_NO, OUTPUT);
  digitalWrite(HEATING_NO, HIGH);
  pinMode(PUMP_NO, OUTPUT);
  digitalWrite(PUMP_NO, HIGH);

  digitalWrite(BYPASS_NC, LOW);

}

void loop() {
  // put your main code here, to run repeatedly:

  time_ms = millis();

  int event = left_button.poll();

//  if (event != NONE){
//    Serial.println(event);
//  }
  
  if(event==PRESS){
    if(!brew_active){
      bias_off = 0;
      bias_on = 1;
      bias_on_time = time_ms;
        brew_active = 1;
      analogWrite(LEFT_BUTTON_LED, DEFAULT_BRIGHTNESS);
      digitalWrite(PUMP_NO, LOW);
    }
    else{
      bias_on = 0;
      bias_off = 1;
      bias_off_time = time_ms;
      brew_active = 0;
      analogWrite(LEFT_BUTTON_LED, 0);
      digitalWrite(PUMP_NO, HIGH);
    }
    
  }
  if(time_ms - last_reading_time >= MEASUREMENT_INTERVAL){

    temperature = max.temperature(RNOMINAL, RREF);
    measurement_array[measurement_index] = temperature;
    
    measurement_index++;
    last_reading_time = time_ms;
    
    //Format and print reading to screen
    display.clearDisplay();
    display.setCursor(47, 42);
    int tempi = int(temperature);
    float tempf = temperature-tempi;
    int tempp = int(tempf*10);
    display.print(tempi); display.print("."); display.print(tempp);
    display.display();
    
    //Print time and value to serial
    Serial.print(millis()/1000.0);
    Serial.print(", ");
    Serial.print(temperature);
    Serial.print('\n');

  }

  //Update control variable for PWM period every four measurements
  if(measurement_index >= 4){
    measurement_index = 0;
    float t_av = 0.1*measurement_array[0]+0.2*measurement_array[1]+0.3*measurement_array[2]+0.4*measurement_array[3];
    float dt = (0.2*(measurement_array[1]-measurement_array[0]) + 0.3*(measurement_array[2]-measurement_array[1]) + 0.5*(measurement_array[3]-measurement_array[2]))/(MEASUREMENT_INTERVAL/1000);
    error_integral += 4*SETPOINT - measurement_array[0] - measurement_array[1] - measurement_array[2] - measurement_array[3];

    float control_variable = K_PROPORTIONAL*(SETPOINT - t_av) + K_INTEGRAL*error_integral - K_DERIVATIVE*dt  - K_NFB*feedback  + bias_on*ON_BIAS - bias_off*OFF_BIAS + BIAS;

    #ifdef VERBOSE
    Serial.print("Proportional: ");
    Serial.print(SETPOINT-t_av);
    Serial.print(", Integral: ");
    Serial.print(error_integral);
    Serial.print(", Derivative: ");
    Serial.print(-1*dt);
    Serial.print(", Feedback: ");
    Serial.print(feedback);
    Serial.print(", Control: ");
    Serial.print(control_variable);
    Serial.print("\n");
    #endif

    if(control_variable < MIN_DUTY_CYCLE){
      digitalWrite(HEATING_NO, HIGH);
      duty_cycle = 0;
    }
    else if(control_variable > MAX_DUTY_CYCLE){
      digitalWrite(HEATING_NO, LOW);
      duty_cycle = 1;
    }
    else{
      pwm_active = 1;
      duty_cycle  = control_variable;
      pwm_start_time = time_ms;
      digitalWrite(HEATING_NO, LOW);
    }

    feedback *= NFB_DECAY;
    feedback += duty_cycle;
  }

  //PWM handler
  if(pwm_active && time_ms-pwm_start_time >= duty_cycle*PWM_PERIOD){
    pwm_active = 0;
    digitalWrite(HEATING_NO, HIGH);
  }

  //Forced bias handlers
  if(bias_on && time_ms-bias_on_time >= BIAS_ON_DURATION){
    bias_on = 0;
  }
  if(bias_off && time_ms-bias_off_time >= BIAS_OFF_DURATION){
    bias_off = 0;
  }

}
