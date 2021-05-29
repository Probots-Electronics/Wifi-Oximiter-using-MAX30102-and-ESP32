# Wifi Oximiter using MAX30102 and ESP32
Code for simple WiFi Oximeter that sends BPM &amp; SPO2 values to a mobile app

![Wifi Oximiter using MAX30102 and ESP32](./images/WorkingDemo.JPG)

This repository contains a program (to be flashed on an ESP32, using the Arduino IDE) which allows to create a sensor capable of sensing Oxygen Saturation SPO2 and Heart Pulse Rate. It works using the MAX30102 sensor by Maxim. The data is then sent to a Mobile App through WiFi where it is displayed.

## Idea behind the Program
Maxim has a great sensor for detecting SPO2 - MAX30102. It is cheap, acurate and quite popular. Maxim has also given out detailed alogirthm to convert the raw values from the sensor to actual SPO2 and  BPM Values.

To use this sensor with Arduino, [Sparkfun has a great library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library). Many people have used this library successfully. But I was getting incorrect SP02 values, BPM values were fluctuating using their example code. I have modified it to get better accuracy for my setup.

This code uses the maxim_heart_rate_and_oxygen_saturation() function as per Maxim's algorithm to calculate the SPO2. For BPM, we use sparkfun Example5_HeartRate.ino which gives accurate BPM.

## Hardware

[MAX30102 Pulse Oximeter](https://www.probots.co.in/gy-max30100-pulse-oximeter-heart-rate-sensor-module.html): Any MAX30102 sensor will work, but make sure the sensor and the I2C Signals work on right voltages. There are many poor quality boards which do not confirm to Maxims specs and will give incorrect values and may not work.

[ESP32 NodeMCU Board](https://www.probots.co.in/esp32-wifi-ble-bluetooth-4-0-iot-development-nodemcu-board-38-pin.html): Use ESP32 Only. Arduino UNO will not be able to run the Maxim Algorithm due to memory limitation.

### Connections
The Connections are simple and straight forward.


ESP32 | VL53L1X board
:------------: | :-------------:
3V3|VIN
GND|GND
SDA (GPIO21)|SDA
SCL (GPIO22)|SCL


## How to modify the code to your requirement

