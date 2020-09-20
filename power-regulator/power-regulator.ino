#include <TM1637.h>
#define MINUTES 14
#define SECONDS 59

#define LEFT_BUTTON_PIN 7
#define RIGHT_BUTTON_PIN 9
#define SIGNAL_PIN 11
#define RIGHT 1
#define LEFT 2
#define MIDDLE 3

// Instantiation and pins configurations
// Pin 3 - > DIO
// Pin 2 - > CLK
TM1637 tm1637(2, 3);
int leftButtonState = 0;
int rightButtonState = 0; 
bool isAnomalyOn = true;
int lastSwitchPosition;
int newSwitch = 0;

void setup()
{
    tm1637.init();
    tm1637.setBrightness(1);
    tm1637.colonOn();
    tm1637.display("----");

    pinMode(LEFT_BUTTON_PIN, INPUT);
    pinMode(RIGHT_BUTTON_PIN, INPUT);

    leftButtonState = digitalRead(LEFT_BUTTON_PIN);
    rightButtonState = digitalRead(RIGHT_BUTTON_PIN);
    if(leftButtonState == HIGH){
      lastSwitchPosition = LEFT;
    }
    if(rightButtonState == HIGH){
      lastSwitchPosition = RIGHT;
    }
    else{
      lastSwitchPosition = MIDDLE;
    }

    Serial.begin(9600);
    Serial.print("lastSwitchPosition: "); Serial.println(lastSwitchPosition);

    turnAnomalyOn();
}

void loop()
{
  if(isAnomalyOn == false){
    // timer goes
    for (int minutes = MINUTES ; minutes >= 0; minutes--) {
        for (int seconds = SECONDS; seconds >= 0 ; seconds--) {
            delay(1000);          // Waits for 1 second = 1000 millisecond.
            tm1637.display(seconds + minutes * 100);

            if(detectSwitchChanges()){
              minutes = MINUTES;
              seconds = SECONDS;
            }   
        }
    }
    turnAnomalyOn();
  }
  else{
    tm1637.display("----");
    //digitalWrite(SIGNAL_PIN, HIGH);
  }
  if(detectSwitchChanges()){
    turnAnomalyOff();
  }
}

int checkSwitchState(){
  bool isRightSwitch = (digitalRead(RIGHT_BUTTON_PIN) == HIGH);
  bool isLeftSwitch = (digitalRead(LEFT_BUTTON_PIN) == HIGH);
  bool stateChanged = (isLeftSwitch && lastSwitchPosition != LEFT) || (isRightSwitch && lastSwitchPosition != RIGHT);

  if(stateChanged){
   if(isLeftSwitch){
     return LEFT;
   }
   else{
     return RIGHT;
   }
  }
  return 0;
}

bool detectSwitchChanges(){
  newSwitch = checkSwitchState();
  if(newSwitch != 0){
    // Turn off anomaly if switch was toggled
    Serial.println("Changes happened");
    Serial.print("Was switched from "); Serial.print(lastSwitchPosition);Serial.print(" to "); Serial.println(newSwitch);
    lastSwitchPosition = newSwitch;
    return true;
  }
  return false;
}

void turnAnomalyOn(){
  isAnomalyOn = true;
  digitalWrite(SIGNAL_PIN, HIGH);
}

void turnAnomalyOff(){
  isAnomalyOn = false;
  digitalWrite(SIGNAL_PIN, LOW);
}
