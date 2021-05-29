#include <Wire.h>
#include <Blynk.h>
#include <WiFi.h> 
#include <BlynkSimpleEsp32.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
 
#define REPORTING_PERIOD_MS 1000 // frequency of updates sent to blynk app in ms
 
char auth[] = "";             // You should get Auth Token in the Blynk App.
char ssid[] = "";                              // Your WiFi credentials.
char pass[] = "";
 
uint32_t tsLastReport = 0;  //stores the time the last update was sent to the blynk app

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value calcualated as per Maxim's algorithm
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

byte pulseLED = 2; //onboard led on esp32 nodemcu
byte readLED = 19; //Blinks with each data read 

long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute; //stores the BPM as per custom algorithm
int beatAvg = 0, sp02Avg = 0; //stores the average BPM and SPO2 
float ledBlinkFreq; //stores the frequency to blink the pulseLED

 
void setup()
{ 
  ledcSetup(0, 0, 8); // PWM Channel = 0, Initial PWM Frequency = 0Hz, Resolution = 8 bits
  ledcAttachPin(pulseLED, 0); //attach pulseLED pin to PWM Channel 0
  ledcWrite(0, 255); //set PWM Channel Duty Cycle to 255
  
  Blynk.begin(auth, ssid, pass);
  Serial.begin(115200);
  
  Serial.print("Initializing Pulse Oximeter..");
  
    // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }


  /*The following parameters should be tuned to get the best readings for IR and RED LED. 
   *The perfect values varies depending on your power consumption required, accuracy, ambient light, sensor mounting, etc. 
   *Refer Maxim App Notes to understand how to change these values
   *I got the best readings with these values for my setup. Change after going through the app notes.
   */
  byte ledBrightness = 50; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}
 
void loop()
{
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
  
  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data
  
    redBuffer[i] = particleSensor.getIR();
    irBuffer[i] = particleSensor.getRed();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  
    Serial.print(F("red: "));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F("\t ir: "));
    Serial.println(irBuffer[i], DEC);
  }
  
  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  
  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    Blynk.run();
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
  
    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data
    
      digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read
    
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      //Uncomment these statements to view the raw data during calibration of sensor.
      //When uncommented, beatsPerMinute will be slightly off.
      /*Serial.print(F("red: "));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F("\t ir: "));
      Serial.print(irBuffer[i], DEC);
      Serial.print(F("\t HR="));
      Serial.print(heartRate, DEC);
      Serial.print(F("\t"));
      Serial.print(beatAvg, DEC);
      
      Serial.print(F("\t HRvalid="));
      Serial.print(validHeartRate, DEC);
      
      Serial.print(F("\t SPO2="));
      Serial.print(spo2, DEC);
      
      Serial.print(F("\t SPO2Valid="));
      Serial.println(validSPO2, DEC);*/

      long irValue = irBuffer[i];

      //Calculate BPM independent of Maxim Algorithm. 
      if (checkForBeat(irValue) == true)
      {
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();
      
        beatsPerMinute = 60 / (delta / 1000.0);
        beatAvg = (beatAvg+beatsPerMinute)/2;

        if(beatAvg != 0)
          ledBlinkFreq = (float)(60.0/beatAvg);
        else
          ledBlinkFreq = 0;
        ledcWriteTone(0, ledBlinkFreq);
      }
      if(millis() - lastBeat > 10000)
      {
        beatsPerMinute = 0;
        beatAvg = (beatAvg+beatsPerMinute)/2;
        
        if(beatAvg != 0)
          ledBlinkFreq = (float)(60.0/beatAvg);
        else
          ledBlinkFreq = 0;
        ledcWriteTone(0, ledBlinkFreq);
      }
    }
  
    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    
    Serial.print(beatAvg, DEC);
    
    Serial.print(F("\t HRvalid="));
    Serial.print(validHeartRate, DEC);
    
    Serial.print(F("\t SPO2="));
    Serial.print( sp02Avg , DEC);
    
    Serial.print(F("\t SPO2Valid="));
    Serial.println(validSPO2, DEC);

    //Calculates average SPO2 to display smooth transitions on Blynk App
    if(validSPO2 == 1 && spo2 < 100 && spo2 > 0)
    {
      sp02Avg = (sp02Avg+spo2)/2;
    }
    else
    {
      spo2 = 0;
      sp02Avg = (sp02Avg+spo2)/2;;
    }

     //Send Data to Blynk App at regular intervals
    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    { 
      Blynk.virtualWrite(V3, beatAvg);
      Blynk.virtualWrite(V4, sp02Avg);
    
      tsLastReport = millis();
    }
  }
}
