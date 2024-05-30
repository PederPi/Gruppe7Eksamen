/***********************************************************
Authors: Hannah Helena Bidstrup og Jonas Peder Iversen
Rover Project, IT-Teknolog
***********************************************************/


//Libraries:
////////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
////////////////////////////////////////////////////////////////////////////////////


//Pin overview (only change pins here and never in the code!):
////////////////////////////////////////////////////////////////////////////////////
//Radio pins:
#define CE_pin 12
#define CSN_pin 13
//Joystick pins:
//A0: joystick x_axis
//A1: joystick y_axis
#define joystick_switch_pin 2 //Not in use
//SPI pins:
#define MISO_pin 50
#define MOSI_pin 51
#define SCK_pin  52
////////////////////////////////////////////////////////////////////////////////////


//Variable/constants/structures defines and declarations :
////////////////////////////////////////////////////////////////////////////////////
//Transmit interval, this interval is achieved by using delay in the loop function:
const int transmit_interval_milliseconds = 50;
//Radio address:
const byte radio_address[6] = "00001";
//Serial Baud rates:
#define serial0_baud_rate 9600
//Data structure for transmitting several data points at a time, in this case 2 different directional data points from the joystick:
struct radio_data_package{
  int x = 0;
  int y = 90;
};
////////////////////////////////////////////////////////////////////////////////////


//Object creation:
////////////////////////////////////////////////////////////////////////////////////
//Data package object:
radio_data_package radio_data;
//Radio object:
RF24 radio(CE_pin, CSN_pin);
////////////////////////////////////////////////////////////////////////////////////


//Main functions (setup and loop):
////////////////////////////////////////////////////////////////////////////////////
void setup() {
  init_serial_connection();
  init_joystick();
  init_radio();
}

void loop() {
  get_joystick_input();
  transmit_data();
  delay(transmit_interval_milliseconds);
}
////////////////////////////////////////////////////////////////////////////////////


//Radio handling functions:
////////////////////////////////////////////////////////////////////////////////////
//Initialize the radio:
void init_radio(){
  radio.begin();
  radio.openWritingPipe(radio_address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}
//Transmit the joystick data to the rover:
void transmit_data(){
  radio.write(&radio_data, sizeof(radio_data_package));
}
////////////////////////////////////////////////////////////////////////////////////


//Serial handling function:
////////////////////////////////////////////////////////////////////////////////////
//Initialize the serial connection for communication to a pc:
void init_serial_connection(){
  Serial.begin(serial0_baud_rate);
}
////////////////////////////////////////////////////////////////////////////////////


//Joystick handling functions:
////////////////////////////////////////////////////////////////////////////////////
//Initialize the joystick, making the joystick x and y axis use A0 and A1 as inputs:
void init_joystick(){
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}
//Read the current position of the joystick and update the variables in the data structure:
void get_joystick_input(){
  radio_data.x = map(A0, 0, 1023, -100, 100);
  radio_data.y = map(A1, 0, 1023, 135, 45);
}
////////////////////////////////////////////////////////////////////////////////////