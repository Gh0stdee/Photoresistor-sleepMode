#include <Arduino.h>
#define lightPin 4

int usFactor = 1000000;  //us to s multiplier         
int detectGap = 5;       //time between checking of light sensor values
bool espState = true;    //set ESP to be On for default

void powerDown()
{
  if (espState)          //if ESP is already "off", no need to run the code during lightLevel check
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
  Serial.begin(9600);             //set up serial communication
  pinMode(lightPin, INPUT);       //configure photoresistor pin
}

void loop()
{
  int light = analogRead(lightPin);                     //read the photoresistor module value
  int lightLevel = map(light, 0, 4095, 0, 15);          //maping the lightLevel into 16 values
  esp_sleep_enable_timer_wakeup(detectGap * usFactor);  //wake up to do lightLevel checking at a regular interval 
  
//lightLevel Checks

  //if light level is low: Go to deep sleep mode
  if (lightLevel <= 11)                             
  {
    powerDown();
  }

  //if the ESP is "off" and light level is high: Do not go back to deep sleep mode
  else if (lightLevel > 11 && !espState)
  {
    powerUp();
  }
 
}