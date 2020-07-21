#include <SoftwareSerial.h>
#include <limits.h>
#include <SPI.h>
#include <SD.h>

//#define DEBUG
//#define DETAIL
#define NANO

#ifdef UNO
  SoftwareSerial mySerial(8, 9);
  const byte SPEAKER_PIN = 10;
  const byte LED_PIN = 12;
  const byte VOLTAGE_PIN = A1;
#endif

#ifdef NANO
  SoftwareSerial mySerial(7, 8);
  const byte SPEAKER_PIN = 4;
  const byte LED_PIN = 2;
  const byte VOLTAGE_PIN = A6;
#endif

#define WIFI_SERIAL    mySerial

File logFile;
const int ON_DURATION = 100;
// максимальный заряд аккумулятора
float max_v = 4.20; 
// минимальный заряд аккумулятора
float min_v = 2.75; 
unsigned long lastPeriodStart = millis();
int periodDuration = INT_MAX;
bool isSignaling = false;
String inString = "";    // string to hold input
String logText = "";

void setup() {
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  // открываем последовательный порт для мониторинга действий в программе
  // и передаём скорость 115200 бод
#ifdef DEBUG
  Serial.begin(4800);
  while (!Serial) {
    // ждём, пока не откроется монитор последовательного поcрта
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
#ifdef DEBUG
  Serial.print("WIFI_SERIAL init OK\r\n");
#endif

  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    tone(SPEAKER_PIN, 550, 1000);
    delay(1000);
    signalOff();
  }
  else{
    for(int i = 0; i < 2; i++){
      signalOn();
      delay(100);
      signalOff();
      delay(100);
    }
  }
  delay(2000);
}

void loop() {
  process();
}

void process() {
  if (WIFI_SERIAL.available()) {
    int inChar = WIFI_SERIAL.read();
    Serial.write(inChar);
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    if (inChar == '\n') {
      int strength = constrain(inString.toInt(), 0, 1000);
      periodDuration = calculatePeriodDuration(strength);

      Serial.print("Logging :");
      String debugText = String(strength);
      debugText = debugText + " : ";
      debugText = debugText + String(logText.length());
      Serial.println(debugText);
      int textLength = logText.length();
      if(textLength < 20){
        logText = logText + String(millis()) + " : " + String(strength) + "\n";
      }
      else{
        textLength = logText.length();
        logFile = SD.open("log.txt", FILE_WRITE);
        // if the file opened okay, write to it:
        if (logFile) {
          Serial.println("Writing to log.txt...");
          char charBuf[textLength];
          logText.toCharArray(charBuf, textLength);
          logFile.write(charBuf);
          logFile.close();
          logText = "\n";
        }
      }

#ifdef DEBUG
      Serial.print("Strength:");
      Serial.println(strength);
      Serial.print("Duration: ");
      Serial.println(periodDuration);
#endif

      // clear the string for new input:
      inString = "";
      // log to sd if possible
      logData(String(strength));

      float perc = getVoltagePercentage();
    }
  }


  if (isTimeToSignal())
  {
#ifdef DEBUG
#ifdef DETAIL
    Serial.println("Signaling...");
#endif
#endif
    lastPeriodStart += periodDuration;
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

void signalOn() {
  tone(SPEAKER_PIN, 750, ON_DURATION);
#ifdef DEBUG
#ifdef DETAIL
  Serial.println("LED_PIN: ON");
#endif
#endif
  digitalWrite(LED_PIN, HIGH);
}

void signalOff() {
#ifdef DEBUG
#ifdef DETAIL
  Serial.println("LED_PIN: OFF");
#endif
#endif
  digitalWrite(LED_PIN, LOW);
}

int calculatePeriodDuration(int strength) {
  // strength : [0:1000]

  if (strength < 4) {
    return INT_MAX;
  }

  double product = strength;
  double timesPerSecond = product / 100.0; // От 0 до 5
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

bool isTimeToSignal() {
  return !isSignaling && millis() - lastPeriodStart >= periodDuration;
}

bool isTimeToStopSignal() {
  return isSignaling && millis() - lastPeriodStart >= ON_DURATION;
}

void logData(String text){
  Serial.print("Logging :");
  Serial.println(text + " : " + logText.length());
  if(logText.length() < 200){
    logText += text + "\n";
  }
  else{
    logFile = SD.open("log.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (logFile) {
      Serial.print("Writing to log.txt...");
      char charBuf[logText.length()];
      logText.toCharArray(charBuf, logText.length());
      logFile.write(charBuf);
      logFile.close();
      logText = "";
    }
  }
}

float getVoltagePercentage(){
  float pinValue = analogRead(VOLTAGE_PIN);
  Serial.println(pinValue);
  float Vbat = (pinValue * 1.1) / 1023;
  float del = 0.091; // R2/(R1+R2)  0.99кОм / (9.88кОм + 0.99кОм)
  float Vin = Vbat / del;
  // уровень заряда в процентах
  int proc = ((Vin - min_v) / (max_v - min_v)) * 100;
  // вывод данных в монитор порта
  Serial.println(String(Vin) + " - " + String(proc));
  return proc;
}

