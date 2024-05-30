###################################################################
#Authors Hannah Helena Bidstrup og Jonas Peder Iversen
###################################################################

#Libraries
###################################################################
import serial
import time 
import datetime
import requests
from picamera2 import Picamera2, Preview
from libcamera import controls 

#Declarations:
###################################################################
SERVER_URL = 'http://192.168.0.103:80/db-insert.php'
BAUDRATE = 9600

#Initializes:
###################################################################
#Initialize the serial connection to the arduino:
serial_to_arduino = serial.Serial (port="/dev/ttyS0", baudrate = BAUDRATE)
time.sleep(0.1)

#Initialize the camera connected to the raspberry pi:
pi_camera = Picamera2()
pi_camera.set_controls({"AfMode": controls.AfModeEnum.Continuous}) 
pi_camera.start()
time.sleep(2)


#Script loop:
###################################################################
while True:
    #Read data from arduino
    received_data = serial_to_arduino.read_until().decode('ascii')  # Read until an expected sequence is found (?\n? by default)
   
    #Create image file name:
    current_datetime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    strFilename = "images/"+current_datetime+".jpg"
    
    #capture image:
    pi_camera.capture_file(strFilename)
    
    #Post data to server:
    post_data = {"password": "Kode1234!", "temperature_value":received_data.rstrip("\n"), "temperature_dateTime":current_datetime, "photo_dateTime":current_datetime}
    files = {"file-image": open(strFilename, "rb")}
    requestsResult = requests.post(SERVER_URL, files = files, data = post_data)
    
    #Write to the arduino that the data have been uploaded:
    arduino_message = bytes(current_datetime + "\n", 'ascii')
    serial_to_arduino.write(arduino_message)

    #Delay
    time.sleep(60)

