# Wifi Oximiter using MAX30102 and ESP32
Code for simple WiFi Oximeter that sends BPM &amp; SPO2 values to a mobile app

![Wifi Oximiter using MAX30102 and ESP32](./images/WorkingDemo.JPG)

This repository contains a program (to be flashed on an ESP32, using the Arduino IDE) which allows to create a sensor capable of sensing Oxygen Saturation SPO2 and Heart Pulse Rate. It works using the MAX30102 sensor by Maxim. The data is then sent to a Mobile App through WiFi where it is displayed.

## Idea behind the Program
Maxim has a great sensor for detecting SPO2 - MAX30102. It is cheap, acurate and quite popular. Maxim has also given out detailed alogirthm to convert the raw values from the sensor to actual SPO2 and  BPM Values.

To use this sensor with Arduino, [Sparkfun has a great library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library). Many people have used this library successfully. But I was getting incorrect SP02 values and BPM values were fluctuating when using their example code. I have modified it to get better accuracy for my setup.

This code uses the maxim_heart_rate_and_oxygen_saturation() function as per Maxim's algorithm to calculate the SPO2. For BPM, we use code from sparkfun Example5_HeartRate.ino which gives accurate BPM.

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

## How to get better accuracy from the sensor
The MAX30102 sensor is capable of giving very accurate readings, but only after proper calibration for each hardware and mounting setup. 

To get the best accuracy you have to ensure the following - 

* you will never get accurate results if you pres and hold the sensor between your fingers!!!! The sensor has to be mounted with moderate pressure onto the finger using any mechanism that puts constant pressure on the sensor. I found using generic rubber band works well.
* the sensor is very sensitive to ambient light so you have to shield it 100%. I got good results by using tape and wrapping it around the finger.
* the sp02 algorithm uses a lookup table to calculate the SP02 based on the raw sensor values as it was developed to run on an Arduino UNO with limited memory and processing power. I used an ESP32 and changed the sp02 calculation in sp02_algorithm.cpp(find this file in the Sparkfun Library) to - 

```n_spo2_calc =103.0-(17.0*n_ratio_average/100.0);```

If you are building a medical device, you have to calibrate the sensor by comparing readings with another calibrated sensor. Maxim has detialed App Notes which explain this and you will get the latest versions on thier website.
