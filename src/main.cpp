#include <Arduino.h>

#define wakePin 2
#define lightPin 4

int usFactor = 60000000;   //us to minutes multiplier         
int detectGap = 10;        //minutes between checking of light sensor values
bool espState = false;    //set ESP to be Off for default
int count = 0;            //Resolve wake up bug: light becoming 4095 everytime after wake up

void blink()                                                  //builtin LED blink for half a second
{
  digitalWrite(LED_BUILTIN,1); 
  delay(500);                                                //keeping the LED on for half a second
  digitalWrite(LED_BUILTIN,0);
}

void powerDown()
{
  if (espState)  //if ESP is "off" (i.e. wake up due to timer), no need to run the state change code
  {
    Serial.println("Going to sleep...");
    espState = false;
  }
  esp_deep_sleep_start();
}

void powerUp()
{
  espState = true;
  Serial.println("Waking up");
}


void setup()
{
  Serial.begin(19200);              //set up serial communication
  pinMode(lightPin, INPUT);         //configure photoresistor pin
  pinMode(wakePin, INPUT_PULLDOWN); //configure wakePin to be input and default low  
  blink();                          //indicate wake up state
  //Pin Voltage
  esp_sleep_enable_ext0_wakeup((gpio_num_t)wakePin,1);  //wake up ESP32 when reading on wakePin is high
  //Timer
  esp_sleep_enable_timer_wakeup(detectGap * usFactor);  //wake up to do lightLevel checking at a regular interval 
}

void loop()
{ 

  int light1 = analogRead(lightPin);                     //read the photoresistor module value
  int light2 = analogRead(lightPin);
  int avgLight = (light1 + light2)/2;
  int lightLevel = map(avgLight, 0, 4095, 0, 15);        //maping the lightLevel into 16 values
  Serial.println(avgLight);
  delay(1000);
  
  //lightLevel Checks

  //if light level is low AND 
  //case 1: ESP is "off" (i.e. wake up due to timer) OR case 2: ESP32 is on (i.e. forget to enter sleep mode after use)
  //Both cases: Go to deep sleep mode
  if (lightLevel <= 11)                             
  {
    powerDown();
  }

  //if the ESP is "off" (i.e. wake up due to timer) AND light level is high: Do not go back to deep sleep mode
  //This is done because the sensor will read a high value as default after wake up
  //count is used to make sure the sensor detect a high value before running powerUp()
  //powerUp() will not execute if the device is already on 
  else if (lightLevel > 11 && !espState && count)
  {
    powerUp();
  }
 count++;
 
}