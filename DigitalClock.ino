#include <LiquidCrystal.h>
#define LCD_RS A4
#define LCD_E A5
#define LCD_DB4 4
#define LCD_DB5 5
#define LCD_DB6 6
#define LCD_DB7 7
#define READ_BRIGHTNESS_VALUE A3
#define WRITE_BRIGHTNESS_VALUE 11
#define READ_TEMPERATURE A2
#define B1 2
#define B2 3
#define B3 12
#define B4 13
#define ALARM_SOUND_BUTTON 8
#define NOTE_C4  262
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262
#define SNOOZE_LENGTH 300000

LiquidCrystal lcd(LCD_RS,
                  LCD_E,
                  LCD_DB4,
                  LCD_DB5,
                  LCD_DB6,
                  LCD_DB7);
int loopPerformedTimeInterval = 100;
unsigned long lastTimeLoopPerformed = 0;
bool isCelcius = true;
bool is12HourFormat = true;
bool displayEmptyAfterBlink;
int clockHour = 13;
int clockMinute = 0;
int clockSecond = 0;
bool checkClockForSetup;
bool setClock = false;
bool isClockMinuteToSet;
bool skipClockAdjust;
int alarmHour = 13;
int alarmMinute = 5;
bool checkAlarmForSetup;
bool setAlarm = false;
bool isAlarmMinuteToSet;
bool skipAlarmAdjust;
int melodies[] = {NOTE_C4,
                  NOTE_G3,
                  NOTE_G3,
                  NOTE_A3,
                  NOTE_G3,
                  0,
                  NOTE_B3,
                  NOTE_C4,
                  0};
int melodyIndex = 0;
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4, 40};
unsigned long lastTonePerformed = 0;
double tonePerformingInterval = 0;
bool isAlarmOn = true;
bool checkAlarmForCancel;
bool snoozeAlarm = false;
unsigned long snoozingStarted;
bool isAlarmRinging = false;
int b1State;
int b1LastState = 0;
unsigned long b1StartPressedTime;
unsigned long b1HoldTime;
int b2State;
int b2LastState = 0;
unsigned long b2StartPressedTime;
unsigned long b2HoldTime;
int b4State;
int b4LastState = 0;
unsigned long b4StartPressedTime;
unsigned long b4HoldTime;

void setup(){
  Serial.begin(9600);
  pinMode(WRITE_BRIGHTNESS_VALUE, OUTPUT);
  pinMode(B1, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(B1),
                  releaseB1, FALLING);
  pinMode(B2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(B2),
                  releaseB2, FALLING);
  pinMode(B3, OUTPUT);
  pinMode(B4, OUTPUT);
  lcd.begin(16, 2);
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void loop(){
  int elapsedTime = millis() - lastTimeLoopPerformed;
  if(elapsedTime >= loopPerformedTimeInterval){
    checkB1();
    checkB2();
    checkB3();
    checkB4();
    if(snoozeAlarm){
      checkSnoozingTime();
    }
    changeBackgroundBrightness();
    displayTemperature();
    if(isAlarmOn){
      checkIfClockAndAlarmTimeEqual();
    }
    if(setClock){
      clockSetup();
    }
    else{
      displayClock();
    }
    if(setAlarm){
      alarmSetup();
    }
    else{
      displayAlarm();
    }
    lastTimeLoopPerformed = millis();
  }
  if(isAlarmRinging && !snoozeAlarm){
    ringTheAlarm();
  }
}

void checkSnoozingTime(){
  int elapsedTime = millis() - snoozingStarted;
  if(elapsedTime >= SNOOZE_LENGTH){
    snoozeAlarm = false;
    melodyIndex = 0;
  }
}

void ringTheAlarm(){
  int elapsed = millis() - lastTonePerformed;
  if(elapsed > tonePerformingInterval){
    double noteDuration = (double)430 / noteDurations[melodyIndex];
    tone(8, melodies[melodyIndex], noteDuration);
    tonePerformingInterval = noteDuration * 1.25;
    lastTonePerformed = millis();
    melodyIndex++;
    if(melodyIndex == 9){
      melodyIndex = 0;
    }
  }
}

void releaseB1(){
  skipClockAdjust = true;
}

void releaseB2(){
  skipAlarmAdjust = true;
}

ISR(TIMER1_COMPA_vect){
  if(!setClock){
    clockSecond++;
    checkTime();
  }
}

void checkIfClockAndAlarmTimeEqual(){
  if((clockHour == alarmHour) && (clockMinute == alarmMinute)){
    isAlarmOn = false;
    isAlarmRinging = true;
  }
}

void displayAlarmHour(){
  lcd.setCursor(8, 1);
  if(is12HourFormat){
    if(alarmHour >= 12){
      int pmFormatHour = alarmHour - 12;
      if(pmFormatHour < 10){
        lcd.print("0");
      }
      lcd.print(pmFormatHour);
    }
    else{
      if(alarmHour < 10){
        lcd.print("0");
      }
      lcd.print(alarmHour);
    }
  }
  else{
    if(alarmHour < 10){
      lcd.print("0");
    }
    lcd.print(alarmHour);
  }
}

void displayAlarmMinute(){
  lcd.setCursor(11, 1);
  if(alarmMinute < 10){
    lcd.print("0");
  }
  lcd.print(alarmMinute);
}

void displayAlarmTimeFormatIndicator(){
  lcd.setCursor(14, 1);
  if(is12HourFormat){
    if(alarmHour >= 12){
      lcd.print("PM");
    }
    else{
      lcd.print("AM");
    }
  }
  else{
    lcd.print("  ");
  }
}

void displayClockHour(){
  lcd.setCursor(0, 0);
  if(is12HourFormat){
    if(clockHour >= 12){
      int pmFormatHour = clockHour - 12;
      if(pmFormatHour < 10){
        lcd.print("0");
      }
      lcd.print(pmFormatHour);
    }
    else{
      if(clockHour < 10){
        lcd.print("0");
      }
      lcd.print(clockHour);
    }
  }
  else{
    if(clockHour < 10){
      lcd.print("0");
    }
    lcd.print(clockHour);
  }
}

void displayEmptyClockHour(){
  lcd.setCursor(0, 0);
  lcd.print("  ");
}

void displayClockMinute(){
  lcd.setCursor(3, 0);
  if(clockMinute < 10){
    lcd.print("0");
  }
  lcd.print(clockMinute);
}

void displayEmptyClockMinute(){
  lcd.setCursor(3, 0);
  lcd.print("  ");
}

void displayClockTimeFormatIndicator(){
  lcd.setCursor(6, 0);
  if(is12HourFormat){
    if(clockHour >= 12){
      lcd.print("PM");
    }
    else{
      lcd.print("AM");
    }
  }
  else{
    lcd.print("  ");
  }
}

void displayClock(){
  displayClockHour();
  lcd.print(":");
  displayClockMinute();
  displayClockTimeFormatIndicator();
}

void checkTime(){
  if(clockSecond == 60){
    clockSecond = 0;
    clockMinute++;
    if(clockMinute == 60){
      clockMinute = 0;
      clockHour++;
      if(clockHour == 24){
        clockHour = 0;
      }
    }
  }
}

void displayAlarmState(){
  lcd.setCursor(3, 1);
  if(isAlarmOn){
    lcd.print("ON");
    lcd.setCursor(6, 1);
    lcd.print(" ");
    lcd.setCursor(5, 1);
  }
  else{
    lcd.print("OFF");
  }
  lcd.print(")");
}

void displayEmptyAlarmHour(){
  lcd.setCursor(8, 1);
  lcd.print("  ");
}

void displayEmptyAlarmMinute(){
  lcd.setCursor(11, 1);
  lcd.print("  ");
}

void displayAlarm(){
  lcd.setCursor(0, 1);
  lcd.print("AL(");
  displayAlarmState();
  displayAlarmHour();
  lcd.print(":");
  displayAlarmMinute();
  displayAlarmTimeFormatIndicator();
}

void displayTemperature(){
  lcd.setCursor(14, 0);
  lcd.print((char)176);
  float degree = (float)analogRead(READ_TEMPERATURE);
  int degreeInt;
  if(isCelcius){
    degreeInt = round(celciusFormatDegree(degree));
    lcd.print("C");
  }
  else{
    degreeInt = round(fahrenheitFormatDegree(degree));
    lcd.print("F");
  }
  int length = String(degreeInt).length();
  lcd.setCursor(13 - length, 0);
  lcd.print(degreeInt);
  lcd.setCursor(10, 0);
  for(int i = 0; i < (3 - length); i++){
    lcd.print(" ");
  }
}

void changeBackgroundBrightness(){
  int brightnessValue = analogRead(READ_BRIGHTNESS_VALUE);
  brightnessValue = map(brightnessValue, 0, 1023, 0, 255);
  analogWrite(WRITE_BRIGHTNESS_VALUE, brightnessValue);
}

float celciusFormatDegree(float degree){
  float celcius = degree / 1024;
  celcius = (((celcius * 5) - 0.5) * 100);
  return celcius;
}

float fahrenheitFormatDegree(float degree){
  float fahrenheit = celciusFormatDegree(degree);
  fahrenheit = ((fahrenheit * 1.8) + 32);
}

void checkB1(){
  if(!setClock){
    b1State = digitalRead(B1);
    if(b1State == HIGH){
      if(b1LastState == LOW){
        b1StartPressedTime = millis();
        checkClockForSetup = true;
      }
      if(checkClockForSetup){
        b1HoldTime = millis() - b1StartPressedTime;
        if(b1HoldTime >= 3000){
          if(!setAlarm){
            checkClockForSetup = false;
            skipClockAdjust = false;
            setClock = true;
            isClockMinuteToSet = true;
            displayEmptyAfterBlink = true;
            clockSecond = 0;
          }
          else{
            checkClockForSetup = false;
            lcd.clear();
            lcd.setCursor(2,0);
            lcd.print("Finish Alarm");
            lcd.setCursor(2, 1);
            lcd.print("Setup First!");
            delay(2000);
            lcd.clear();
            displayAlarm();
          }
        }
      }
    }
    else{
      if(b1LastState == HIGH){
        b1HoldTime = millis() - b1StartPressedTime;
        if(b1HoldTime < 3000){
          is12HourFormat = !is12HourFormat;
          displayAlarm();
        }
      }
    }
    b1LastState = b1State;
  }
  else{
    if(digitalRead(B1)){
      if(isClockMinuteToSet && skipClockAdjust){
        isClockMinuteToSet = false;
        skipClockAdjust = false;
        displayEmptyAfterBlink = true;
        displayClockMinute();
      }
      else if(!isClockMinuteToSet && skipClockAdjust){
        setClock = false;
      }
    }
  }
}

void checkB2(){
  if(!setAlarm){
    b2State = digitalRead(B2);
    if(b2State == HIGH){
      if(b2LastState == LOW){
        b2StartPressedTime = millis();
        checkAlarmForSetup = true;
      }
      if(checkAlarmForSetup){
        b2HoldTime = millis() - b2StartPressedTime;
        if(b2HoldTime >= 3000){
          if(!setClock){
            checkAlarmForSetup = false;
            skipAlarmAdjust = false;
            setAlarm = true;
            isAlarmMinuteToSet = true;
            displayEmptyAfterBlink = true;
            isAlarmOn = false;
            displayAlarmState();
          }
          else{
            checkAlarmForSetup = false;
            lcd.clear();
            lcd.setCursor(2,0);
            lcd.print("Finish Clock");
            lcd.setCursor(2, 1);
            lcd.print("Setup First!");
            delay(2000);
            lcd.clear();
            displayClock();
          }
        }
      }
    }
    else{
      if(b2LastState == HIGH){
        b2HoldTime = millis() - b2StartPressedTime;
        if(b2HoldTime < 3000){
          if(!isAlarmRinging){
            isAlarmOn = !isAlarmOn;
          }
          else{
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Cancel Current");
            lcd.setCursor(2, 1);
            lcd.print("Alarm First");
            delay(2000);
            lcd.clear();
            displayClock();
          }
        }
      }
    }
    b2LastState = b2State;
  }
  else{
    if(digitalRead(B2)){
      if(isAlarmMinuteToSet && skipAlarmAdjust){
        isAlarmMinuteToSet = false;
        skipAlarmAdjust = false;
        displayEmptyAfterBlink = true;
        displayAlarmMinute();
      }
      else if(!isAlarmMinuteToSet && skipAlarmAdjust){
        setAlarm = false;
      }
    }
  }
}

void checkB3(){
  if(digitalRead(B3)){
    if(setClock){
      if(isClockMinuteToSet){
        clockMinute++;
        if(clockMinute == 60){
          clockMinute = 0;
        }
      }
      else{
        clockHour++;
        if(clockHour == 24){
          clockHour = 0;
        }
        if(is12HourFormat && (clockHour == 0 || clockHour == 12)){
          displayClockTimeFormatIndicator();
        }
      }
    }
    else if(setAlarm){
      if(isAlarmMinuteToSet){
        alarmMinute++;
        if(alarmMinute == 60){
          alarmMinute = 0;
        }
      }
      else{
        alarmHour++;
        if(alarmHour == 24){
          alarmHour = 0;
        }
        if(is12HourFormat && (alarmHour == 0 || alarmHour == 12)){
          displayAlarmTimeFormatIndicator();
        }
      }
    }
    else{
      isCelcius = !isCelcius;
    }
  }
}

void checkB4(){
  if(isAlarmRinging){
    b4State = digitalRead(B4);
    if(b4State == HIGH){
      if(b4LastState == LOW){
        b4StartPressedTime = millis();
        checkAlarmForCancel = true;
      }
      if(checkAlarmForCancel){
        b4HoldTime = millis() - b4StartPressedTime;
        if(b4HoldTime >= 3000){
          checkAlarmForCancel = false;
          snoozeAlarm = false;
          isAlarmRinging = false;
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("Alarm Canceled");
          delay(2000);
          lcd.clear();
          displayClock();
          displayAlarm();
        }
      }
    }
    else{
      if(b4LastState == HIGH){
        b4HoldTime = millis() - b4StartPressedTime;
        if(b4HoldTime < 3000){
          snoozeAlarm = true;
      	  snoozingStarted = millis();
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("Alarm Snoozed");
          lcd.setCursor(3, 1);
          lcd.print("For 5 Mins");
          delay(2000);
          lcd.clear();
          displayClock();
          displayAlarm();
        }
      }
    }
    b4LastState = b4State;
  }
}

void clockSetup(){
  if(isClockMinuteToSet){
    if(displayEmptyAfterBlink){
      displayEmptyClockMinute();
      displayEmptyAfterBlink = false;
    }
    else{
      displayClockMinute();
      displayEmptyAfterBlink = true;
    }
  }
  else{
    if(displayEmptyAfterBlink){
      displayEmptyClockHour();
      displayEmptyAfterBlink = false;
    }
    else{
      displayClockHour();
      displayEmptyAfterBlink = true;
    }
  }
}

void alarmSetup(){
  if(isAlarmMinuteToSet){
    if(displayEmptyAfterBlink){
      displayEmptyAlarmMinute();
      displayEmptyAfterBlink = false;
    }
    else{
      displayAlarmMinute();
      displayEmptyAfterBlink = true;
    }
  }
  else{
    if(displayEmptyAfterBlink){
      displayEmptyAlarmHour();
      displayEmptyAfterBlink = false;
    }
    else{
      displayAlarmHour();
      displayEmptyAfterBlink = true;
    }
  }
}