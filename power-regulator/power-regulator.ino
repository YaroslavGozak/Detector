#include <GyverTM1637.h>
#define MINUTES 0
#define SECONDS 30
#define ON 1
#define OFF 0

#define NODEMCU

#ifdef NODEMCU

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

GyverTM1637 disp(5,4);
const int BUTTON_PIN = 12;
const char* ssid = "M200";
const char* password = "P@ssword";
const int LED_PIN = 13;

#endif

int buttonState = 0;
bool isAnomalyOn = true;
int lastSwitchPosition;
int newSwitch = -1;

void setup()
{
  Serial.begin(9600);
  delay(10);
  Serial.print("Starting... ");
  disp.clear();
  disp.brightness(7);
  disp.point(true);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  buttonState = digitalRead(BUTTON_PIN);
  if(buttonState == HIGH){
    lastSwitchPosition = ON;
  }
  else{
    lastSwitchPosition = OFF;
  }

  Serial.print("lastSwitchPosition: "); Serial.println(lastSwitchPosition);

  turnAnomalyOn();
}

void loop()
{
  if(isAnomalyOn == false){
    // timer goes
    for (int minutes = MINUTES ; minutes >= 0; minutes--) {
      int seconds = 59;
      if(MINUTES == 0){
        seconds = SECONDS;
      }
        for ( ; seconds >= 0 ; seconds--) {
            delay(1000);          // Waits for 1 second = 1000 millisecond.
            //tm1637.display(seconds + minutes * 100);
            disp.displayClock(minutes, seconds);

           // if(detectSwitchChanges()){
           //   minutes = MINUTES;
           //   seconds = SECONDS;
           // }   
        }
    }
    turnAnomalyOn();
  }
  else{
    //tm1637.display("----");
    disp.displayByte(_dash,_dash,_dash,_dash);
  }
  if(detectSwitchChanges()){
    turnAnomalyOff();
  }
}

bool detectSwitchChanges(){
  newSwitch = checkSwitchState();
  if(newSwitch != -1){
    // Turn off anomaly if switch was toggled
    Serial.println("Changes happened");
    Serial.print("Was switched from "); Serial.print(lastSwitchPosition);Serial.print(" to "); Serial.println(newSwitch);
    lastSwitchPosition = newSwitch;
    return true;
  }
  return false;
}

int checkSwitchState(){
  bool isSwitch = (digitalRead(BUTTON_PIN) == HIGH);
  bool stateChanged = isSwitch != (lastSwitchPosition == ON);

  if(stateChanged){
   return isSwitch;
  }
  return -1;
}



void turnAnomalyOn(){
  isAnomalyOn = true;
  Serial.println("Turning on...");
  #ifdef NODEMCU
    WiFi.softAP(ssid, password); //begin WiFi access point
    digitalWrite(LED_PIN, HIGH);
  #endif
}

void turnAnomalyOff(){
  isAnomalyOn = false;
  Serial.println("Turning off...");
  #ifdef NODEMCU
    WiFi.softAPdisconnect(true);
    digitalWrite(LED_PIN, LOW);
  #endif
 
}
