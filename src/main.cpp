#include <Arduino.h>

#define wakePin 32
#define lightPin 34

int usFactor = 60000000;   //us to minutes multiplier         
int detectGap = 10;        //minutes between checking of light sensor values
bool espState = false;    //set ESP to be Off for default
bool voltageWake;                //Touch pin voltage 
int voltageWake_count;
//int count = 0;            //Resolve wake up bug for old sensor: light becoming 4095 everytime after wake up

//indicate wake up state        
void blink()                                           
{
  digitalWrite(LED_BUILTIN,1); 
  delay(500);                                                //builtin LED blink for half a second
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
  blink();                          
}


void setup()
{
  Serial.begin(19200);              //set up serial communication
  pinMode(lightPin, INPUT);         //configure photoresistor pin
  pinMode(wakePin, INPUT_PULLDOWN); //configure wakePin to be input and default low  
  pinMode(LED_BUILTIN,OUTPUT);
  //Pin Voltage
  esp_sleep_enable_ext0_wakeup((gpio_num_t)wakePin,1);  //wake up ESP32 when reading on wakePin is high
  voltageWake = digitalRead(wakePin);
  voltageWake_count = 1;
  //Timer
  esp_sleep_enable_timer_wakeup(detectGap * usFactor);  //wake up to do lightLevel checking at a regular interval 
}

void loop()
{ 
  //Get pin readings
  voltageWake = digitalRead(wakePin);
  int light1 = analogRead(lightPin);                     //read the photoresistor module value
  int light2 = analogRead(lightPin);
  int avgLight = (light1 + light2)/2;
  int darkness = map(avgLight, 0, 4095, 0, 15);        //maping the lightLevel into 16 values
  Serial.println(avgLight);
  delay(1000);

  //voltage wakeup check
  if (voltageWake && !espState)
  {
    //Activate the device i.e. do not care about photoresistor sensor value

    if (voltageWake_count == 1) //blink once after activation by voltage pin
    {
      blink(); 
      espState = true;
      voltageWake_count +=1;
    }

    // Do something here e.g. Serial.println(" ");

  }

  //lightLevel checks
  //if light level is low AND 
  //case 1: ESP is "off" (i.e. wake up due to timer) OR case 2: ESP32 is on (i.e. forget to enter sleep mode after use)
  //Both cases: Go to deep sleep mode
  else if (darkness > 11) // old sensor:(lightLevel <= 11)                             
  {
    if(voltageWake)
    {
      Serial.println("Please turn on the lights.");
    }
    else
    powerDown();
  }
  //Old sensor code explanation for (lightLevel > 11 && !espState && count):
  //if the ESP is "off" (i.e. wake up due to timer) AND light level is high: Do not go back to deep sleep mode
  //This is done because the sensor will read a high value as default after wake up
  //count is used to make sure the sensor detect a high value before running powerUp()
  //powerUp() will not execute if the device is already on 
 
  else if (darkness < 11 && !espState) 
  {
    powerUp();
  }
 //count++;
 
}