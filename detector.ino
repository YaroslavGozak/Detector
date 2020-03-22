#include <SoftwareSerial.h>
#include <limits.h>

#define DEBUG

SoftwareSerial mySerial(8, 9);
const byte SPEAKER_PIN = 10;
const byte LED_PIN = 12;
const int ON_DURATION = 100;

unsigned long lastPeriodStart = millis();
int periodDuration = INT_MAX;
bool isSignaling = false;
String inString = "";    // string to hold input
int testStrength = 0;

#define WIFI_SERIAL    mySerial
#define WIFI_DEGUG

void setup() {
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  // открываем последовательный порт для мониторинга действий в программе
  // и передаём скорость 115200 бод
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
  // ждём, пока не откроется монитор последовательного порта
  // для того, чтобы отследить все события в программе
  }
  Serial.print("Serial init OK\r\n");
  #endif
  
  // открываем Serial-соединение с Wi-Fi модулем на скорости 115200 бод
  WIFI_SERIAL.begin(115200);
  while (!WIFI_SERIAL) {
  // ждём, пока не откроется монитор последовательного порта
  // для того, чтобы отследить все события в программе
  }
  Serial.print("WIFI_SERIAL init OK\r\n");
}

void loop() {
  process();
}

void process(){
  if (WIFI_SERIAL.available()) {
    int inChar = WIFI_SERIAL.read();
    Serial.write(inChar);
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    if (inChar == '\n') {
      int strength = constrain(inString.toInt(), 0, 1000);
      testStrength++;
      periodDuration = calculatePeriodDuration(strength);

      #ifdef DEBUG
        Serial.print("Strength:");
        Serial.println(strength);
        Serial.print("Duration: ");
        Serial.println(periodDuration);
      #endif
      
      // clear the string for new input:
      inString = "";
    }
  }


  if (isTimeToSignal())
  {
    #ifdef DEBUG
      #ifdef DETAIL
        Serial.println("Signaling...");
      #endif
    #endif
    lastPeriodStart+=periodDuration;
    signalOn();
    isSignaling = true;
  }

  if (isTimeToStopSignal())
  {
    #ifdef DEBUG
      #ifdef DETAIL
        Serial.println("STOP Signaling...");
      #endif
    #endif
    signalOff();
    isSignaling = false;
  }
}

void signalOn(){
  tone(SPEAKER_PIN, 750, ON_DURATION);
  #ifdef DEBUG
    #ifdef DETAIL
      Serial.println("LED_PIN: ON");
    #endif
  #endif
  digitalWrite(LED_PIN, HIGH);
}

void signalOff(){
  #ifdef DEBUG
    #ifdef DETAIL
      Serial.println("LED_PIN: OFF");
    #endif
  #endif
  digitalWrite(LED_PIN, LOW);
}

int calculatePeriodDuration(int strength){
  // strength : [0:1000]

  if(strength < 4){
    return INT_MAX;
  }
  
  double product = strength * 5;
  double timesPerSecond = product / 500.0; // От 0 до 10
  int duration = 1000 / timesPerSecond;

  Serial.print("Prod: ");
  Serial.println(product);
  Serial.print("timesPerSecond: ");
  Serial.println(timesPerSecond);
  Serial.print("duration: ");
  Serial.println(duration);
  
  duration = constrain(duration, ON_DURATION, 2.5 * 1000); 
  duration += 15;

  
  
  return duration;
}

bool isTimeToSignal(){
  return !isSignaling && millis() - lastPeriodStart >= periodDuration;
}

bool isTimeToStopSignal(){
  return isSignaling && millis() - lastPeriodStart >= ON_DURATION;
}
