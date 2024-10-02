#include <Arduino.h>
#define lightPin 4

int usFactor = 1000000;   //us to s multiplier         
int detectGap = 5;        //time between checking of light sensor values
bool espState = false;    //set ESP to be Off for default
int count = 0;            //Resolve wake up bug: light becoming 4095 everytime after wake up

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
  Serial.begin(19200);             //set up serial communication
  pinMode(lightPin, INPUT);       //configure photoresistor pin
  esp_sleep_enable_timer_wakeup(detectGap * usFactor);  //wake up to do lightLevel checking at a regular interval 
}

void loop()
{ 

  int light1 = analogRead(lightPin);                     //read the photoresistor module value
  int light2 = analogRead(lightPin);
  int avgLight = (light1 + light2)/2;
  int lightLevel = map(avgLight, 0, 4095, 0, 15);          //maping the lightLevel into 16 values
  Serial.println(avgLight);
  delay(1000);
//lightLevel Checks

  //if light level is low: Go to deep sleep mode
  if (lightLevel <= 11)                             
  {
    powerDown();
  }

  //if the ESP is "off" and light level is high: Do not go back to deep sleep mode
  //resolved problem: will not run powerUp() by the wake up bug
  else if (lightLevel > 11 && !espState && count)
  {
    powerUp();
  }
 count++;
 
}