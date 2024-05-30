/***********************************************************
Authors: Hannah Helena Bidstrup og Jonas Peder Iversen
Rover Project, IT-Teknolog
***********************************************************/


//Libraries:
////////////////////////////////////////////////////////////////////////////////////
#include <max6675.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <Servo.h>
#include <nRF24L01.h>
#include <RF24.h>
////////////////////////////////////////////////////////////////////////////////////


//Pin overview (only change pins here and never in the code!):
////////////////////////////////////////////////////////////////////////////////////
//SPI pins:
#define MISO_pin 50
#define MOSI_pin 51
#define SCK_pin  52
#define thermocouple_CS_pin 53
//Motor PWM pins:
#define right_Motor_PWN_pin 7             //Right Motor = Motor 1 = Motor A
#define left_Motor_PWN_pin 8              //Left Motor = Motor 2 = Motor B
//Motor direction pins:
#define right_motor_direction_1_pin 4
#define right_motor_direction_2_pin 5
#define left_motor_direction_1_pin 2
#define left_motor_direction_2_pin 3
//OLED display pin:
#define oled_reset 9
//Steering servo pin:
#define steering_servo_pin 6
//Radio pin:
#define CE_pin 13
#define CSN_pin 12
////////////////////////////////////////////////////////////////////////////////////


//Variable/constants/structures defines and declarations :
////////////////////////////////////////////////////////////////////////////////////
//Temperature measurement interval in seconds:
#define temperature_measurement_interval_in_seconds 60
//Loop delay in milliseonds eg. update delay for joystick inputs and motor updates:
#define loop_update_delay_in_milliseconds 100
//Motor frequency:
#define motor_PWM_frequency 25000
//Display variables:
#define screen_width 128 // OLED display width, in pixels
#define screen_height 64 // OLED display height, in pixels
#define screen_address 0x3C
//Variable used by multiple functions for controlling motor frequency and speed:
int ICR4Variable = 0;
//Timer variable for thermocouple reading:
int counter = 0;
//Steering servo angle offset, amount the servo is offset from what it is supposed to be (degrees):
const int steering_servo_offset = 10;
//Throttle deadzone offset, makes sure the rover doesnt jitter back and forth when the joystick is in the resting position:
const int throttle_deadzone_offset = 3; 
//Serial Baud rates:
#define serial0_baud_rate 9600
#define serial1_baud_rate 9600
//Radio address:
const byte radio_address[6] = "00001";
//Radio data package structure:
struct radio_data_package {
  int x = 0;
  int y = 90;
};
radio_data_package radio_data;
//Variables used for steering and throttle:
int x_axis = 0;
int y_axis = 90;
////////////////////////////////////////////////////////////////////////////////////


//Object creation:
////////////////////////////////////////////////////////////////////////////////////
//Thermo couple object:
MAX6675 thermo_couple(SCK_pin, thermocouple_CS_pin, MISO_pin);
//Display object:
Adafruit_SSD1306 display(screen_width, screen_height, &Wire, oled_reset);
//Servo object:
Servo steering_servo;
//Radio object:
RF24 radio(CE_pin, CSN_pin);
////////////////////////////////////////////////////////////////////////////////////


//Main functions (setup and loop):
////////////////////////////////////////////////////////////////////////////////////
void setup(){
  init_serial();
  //init_radio();
  init_motors();
  init_steering_servo();
  init_oled_display();
}

void loop(){
  //get_radio_data();
  update_steering();
  update_motors();
  get_thermo_couple_temperature_and_write_to_raspberry_pi();
  delay(loop_update_delay_in_milliseconds);
}
////////////////////////////////////////////////////////////////////////////////////


//Motor handling functions:
////////////////////////////////////////////////////////////////////////////////////
void init_motors(){
  //PWM motor pins:
  pinMode(right_Motor_PWN_pin, OUTPUT);
  pinMode(left_Motor_PWN_pin, OUTPUT);
  //Motor direction pins:
  pinMode(right_motor_direction_1_pin, OUTPUT);
  pinMode(right_motor_direction_2_pin, OUTPUT);
  pinMode(left_motor_direction_1_pin, OUTPUT);
  pinMode(left_motor_direction_2_pin, OUTPUT);
  //Change pin 7 and 8 to PWM pins:
  analogWrite(right_Motor_PWN_pin, 128);
  analogWrite(left_Motor_PWN_pin, 128);
  //Set hardware PWM frequency:
  set_motor_frequency(motor_PWM_frequency);
}

void set_motor_direction(String direction){
  if(direction == "backwards"){
    digitalWrite(right_motor_direction_1_pin, LOW);
    digitalWrite(right_motor_direction_2_pin, HIGH);
    digitalWrite(left_motor_direction_1_pin, HIGH);
    digitalWrite(left_motor_direction_2_pin, LOW);
  }else if(direction == "forwards"){
    digitalWrite(right_motor_direction_1_pin, HIGH);
    digitalWrite(right_motor_direction_2_pin, LOW);
    digitalWrite(left_motor_direction_1_pin, LOW);
    digitalWrite(left_motor_direction_2_pin, HIGH);
  }
}

void set_motor_speed(int percentage_duty_cycle) {
  int OCR4 = ICR4Variable * ((float)(percentage_duty_cycle / 100.0));
  OCR4B = OCR4;
  OCR4C = OCR4;
}

void set_motor_frequency(int frequency){
  noInterrupts();
  TCCR4A = 0; // clear register
  TCCR4B = 0; // clear register
  TCNT4 = 0; // clear register
  ICR4 = (F_CPU / frequency) / 2;
  ICR4Variable = ICR4;
  TCCR4A = _BV(COM4C1) | _BV(COM4B1)| _BV(WGM41);       //COM4C1 = pin 8, COM4B1 = pin 7
  TCCR4B = _BV(WGM43) | _BV(CS40);
  interrupts();
}

void update_motors(){
  int throttle = abs(x_axis);
  //Control if going backwards, forwards or standing still:
  if(x_axis > throttle_deadzone_offset){
    set_motor_direction("forwards");
    set_motor_speed(throttle);
  }else if(x_axis < -throttle_deadzone_offset){
    set_motor_direction("backwards");
    set_motor_speed(throttle);
  }else{
    set_motor_speed(0);
  }
}
////////////////////////////////////////////////////////////////////////////////////


//Serial handling functions:
////////////////////////////////////////////////////////////////////////////////////
void init_serial(){
  Serial.begin(serial0_baud_rate);
  Serial1.begin(serial1_baud_rate);
}

String get_pi_serial_data(){
  String raspberry_pi_serial_data = Serial1.readStringUntil("\n");
  return raspberry_pi_serial_data;
}
////////////////////////////////////////////////////////////////////////////////////


//Thermo couple handling functions:
////////////////////////////////////////////////////////////////////////////////////
float get_thermo_couple_temperature(){
  return (thermo_couple.readCelsius());
}

void get_thermo_couple_temperature_and_write_to_raspberry_pi(){
  counter += 1;
  if(counter == temperature_measurement_interval_in_seconds * 10){
    float current_temperature = get_thermo_couple_temperature();
    Serial1.print(current_temperature);
    Serial1.print("\n");
    update_oled_display();
    counter = 0;
  }
}
////////////////////////////////////////////////////////////////////////////////////


//Radio handling functions:
////////////////////////////////////////////////////////////////////////////////////
void init_radio(){
  radio.begin();
  radio.openReadingPipe(0, radio_address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void get_radio_data(){
  if (radio.available()) {
    radio.read(&radio_data, sizeof(radio_data_package));
  }
  x_axis = radio_data.x;
  y_axis = radio_data.y;
}
////////////////////////////////////////////////////////////////////////////////////


//Display handling function:
////////////////////////////////////////////////////////////////////////////////////
void init_oled_display(){
  display.begin(SSD1306_SWITCHCAPVCC, screen_address);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("No data");
  display.print("sent yet");
  display.display();
} 

void update_oled_display(){
  String display_text_string = get_pi_serial_data();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Upload OK");
  display.println("");
  display.println(display_text_string);
  display.display();
}
////////////////////////////////////////////////////////////////////////////////////


//Steering servo handling function:
////////////////////////////////////////////////////////////////////////////////////
void init_steering_servo(){
  steering_servo.attach(steering_servo_pin);
}

void update_steering(){
  steering_servo.write(y_axis + steering_servo_offset);
}
////////////////////////////////////////////////////////////////////////////////////