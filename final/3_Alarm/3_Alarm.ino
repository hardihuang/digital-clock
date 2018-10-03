/*
 *Digital Clock V0.2.5 
 *by Hardi Huang
 *
 *update log:
 *  V0.2.5 Oct/3/2018 20:01 added eeprom store alarm data function
 *  V0.2.4 Oct/3/2018 19:06 fixed alarm snooze function and bright flicking bug
 *  V0.2.2 sep/30/2018 11:35 added alarm function 
 *  V0.2.1 sep/29/2018 13:13 added photocell change brightness function
*/


/* 
max7219 matrix display CONNECTIONS:
 *CLK -> 13
 *CS -> 10
 *DIN -> 11
BUTTON CONNECTIONS:
 *arduino internel pull-up
 *btnLeft -> D5
 *btnRight -> D3
 *btnSet -> D4
RTC 1302 CLOCK CONNECTIONS:
 * DAT ->D6
 * CLK ->D7
 * RST ->D8
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
#include <DS1302.h>
#include <EEPROM.h>

int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

namespace {
  const int kCePin   = 8;  //RST
  const int kIoPin   = 6;  //DAT
  const int kSclkPin = 7;  //CLK
  DS1302 rtc(kCePin, kIoPin, kSclkPin);
}//namespace

char alarmLogo[8][32] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
char menuGraph[8][32] =
{
  {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},//1
  {0,0,0,0,0,1,0,1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},//2
  {0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0},//3
  {0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0},//4
  {1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},//5
  {0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0},//6
  {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},//7
  {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0}//8
};

int btnLeft = 0;
int btnRight = 0;
int btnSet = 0;
int timeData[7] = {2018,9,28,18,34,44,4}; //year,month,date,hour,minute,second,day
int alarmData[3] = {17,40,1};//hour,minute,on or off
unsigned long snoozeTimer = millis();
int state = 0; //0==display mode; 1==menu; 2==set time; 3==set alarm; 4==Pomodoro; 5==alarm goes off;
int selectedTime = 3; //which one we are editing right now,same order with timeData
int selectedAlarm = 0;
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};
unsigned long editTimer = millis();
unsigned long dotTimer = millis();
bool dotState = 1;
int buzzPin = 9;
bool alarmCanceled;
int photocellPin = 1;
int photocellReading;
int brightness=1;//1-15
unsigned long alarmBlinkTimer = millis();
bool alarmState = 1;

void setup() {
  tone(buzzPin, 415, 500);
  tone(buzzPin, 415, 500);
  delay(500*1.3);
  tone(buzzPin, 466, 500);
  delay(500*1.3);
  tone(buzzPin, 370, 1000);
  delay(1000*1.3);
  noTone(buzzPin);

  pinMode(3,INPUT_PULLUP);
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);
  Serial.begin(9600); 
  rtc.writeProtect(false);
  rtc.halt(false);
  matrix.setIntensity(10);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);

  fetchAlarmData();
}

void loop() {
  changeBrightness();
  checkAlarm();
  getKey();
  if(state == 0){//display time mode
    if(key == "S"){//go to menu
      state = 1; 
      editTimer = millis(); 
    }
    getTime();
  }else if(state == 1){{//menu mode
    if(key == "L"){//go to set time mode
      state = 2;
      editTimer = millis();
    }else if(key == "R"){//go to set alarm mode
      state = 3;
      editTimer = millis();
    }else if(key == "S"){//exit
      state = 0;  
      editTimer = millis();
    }
  }
  }else if(state == 2){//set time mode
    if(key == "L"){
      minusOneTime();  
      editTimer = millis();
    }else if(key == "R"){
      addOneTime();  
      editTimer = millis();
    }else if(key == "S"){
      editTimer = millis();
      if(selectedTime<4){
        selectedTime++;  
      }else{
        selectedTime = 3;
        state = 0;  
        timeData[5]=0;//reset second
        updateTimeData();
      }
    }
  }else if(state == 3){//set alarm mode
    if(key == "L"){
      minusOneAlarm();  
      editTimer = millis();
      writeAlarmData();
    }else if(key == "R"){
      addOneAlarm();  
      editTimer = millis();
      writeAlarmData();
    }else if(key == "S"){
      editTimer = millis();
      if(selectedAlarm<2){
        selectedAlarm++;  
      }else{
        selectedAlarm = 0;
        state = 0;  
        
      }
      writeAlarmData();
    }
  }else if(state == 5){//alarm goes off
    //BUZZ code in the drawDisplay 
    editTimer = millis();
    if(key == "S"){
      noTone(buzzPin);
      digitalWrite(buzzPin, LOW); 
      snoozeTimer = millis();
      matrix.setIntensity(brightness); 
      state = 0;
    }
  }

  if(state != 0 and millis() - editTimer > 15000){
    state = 0;  
    selectedTime = 3;
  }
  
  drawDisplay();
  
  key = "0";
  delay(100);
}

void getKey(){
  int keyArray[] = {5,3,4};//left, right, set
  for(int i = 0; i < 3; i++){
    if(digitalRead(keyArray[i]) == 0){
      if(debounce[i] == 0){
        key = hexaKeys[i];
        digitalWrite(buzzPin, HIGH); 
        delay(1);
        digitalWrite(buzzPin, LOW); 
        debounce[i] = 3;  
      }else{
        debounce[i] -= 1;  
      }
    }else{
      debounce[i] = 0;
    }
  }
}

void getTime(){
  Time t = rtc.time();
  timeData[0] = t.yr;
  timeData[1] = t.mon;
  timeData[2] = t.date;
  timeData[3] = t.hr;
  timeData[4] = t.min;
  timeData[5] = t.sec;
  timeData[6] = t.day;
}

void drawDisplay(){
  if(state != 5){
    matrix.fillScreen(LOW);
  }
  
  if(state == 0 or state == 2){//display clock mode
    String strHrTime = String(timeData[3]); 
    if(timeData[3]<10){
      strHrTime="0"+strHrTime;
    }
    String strMinTime = String(timeData[4]);
    if(timeData[4]<10){
      strMinTime="0"+strMinTime;
    }
    String strSecTime = String(timeData[5]); 
    if(timeData[5]<10){
      strSecTime="0"+strSecTime;
    }
  //draw hours
    matrix.drawChar(1, 1, strHrTime.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 1, strHrTime.charAt(1),HIGH,LOW, 1);
  //draw minutes
    matrix.drawChar(19, 1, strMinTime.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 1, strMinTime.charAt(1), HIGH, LOW, 1);
  //draw dots
    if(millis()-dotTimer>=500  ){
      if(dotState == 1){
        dotState = 0;
      }else if(dotState == 0 and state == 0){
        dotState = 1;
      }
      dotTimer = millis();  
    }
    // draw dots
    if(state == 0){//normal double blinking dot
      matrix.drawPixel(15,1,dotState);
      matrix.drawPixel(15,2,dotState);
      matrix.drawPixel(16,1,dotState);
      matrix.drawPixel(16,2,dotState);
      matrix.drawPixel(15,5,dotState);
      matrix.drawPixel(15,6,dotState);
      matrix.drawPixel(16,5,dotState);
      matrix.drawPixel(16,6,dotState);
    }else if(state == 2){ 
      if(selectedTime == 3){//left arrow edit hour
        matrix.drawPixel(16,3,1);
        matrix.drawPixel(16,5,1);
        matrix.drawPixel(16,4,1);
        matrix.drawPixel(15,4,1);
      }else if(selectedTime == 4){//right arrow edit minute
        matrix.drawPixel(15,3,1);
        matrix.drawPixel(15,5,1);
        matrix.drawPixel(15,4,1);
        matrix.drawPixel(16,4,1);
      }
    }
  }else if(state == 1){
    for (int i = 0;i<32;i++){
      for(int j=0;j<8;j++){
        matrix.drawPixel(i,j,menuGraph[j][i]);  
      }
    } 
  }else if(state == 3){//set alarm
    String strHrAlarm = String(alarmData[0]); 
    if(alarmData[0]<10){
      strHrAlarm="0"+strHrAlarm;
    }
    String strMinAlarm = String(alarmData[1]);
    if(alarmData[1]<10){
      strMinAlarm="0"+strMinAlarm;
    }
  //draw hours
    matrix.drawChar(1, 1, strHrAlarm.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 1, strHrAlarm.charAt(1),HIGH,LOW, 1);
  //draw minutes
    matrix.drawChar(19, 1, strMinAlarm.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 1, strMinAlarm.charAt(1), HIGH, LOW, 1);
  //draw on off indicator
    if(alarmData[2]==1){
      matrix.drawPixel(31,0,1);
    }else{
      matrix.drawPixel(31,0,0);  
    }
  //draw dot arrow
    if(selectedAlarm == 0){//left arrow edit hour
      matrix.drawPixel(16,3,1);
      matrix.drawPixel(16,5,1);
      matrix.drawPixel(16,4,1);
      matrix.drawPixel(15,4,1);
    }else if(selectedAlarm == 1){//right arrow edit minute
      matrix.drawPixel(15,3,1);
      matrix.drawPixel(15,5,1);
      matrix.drawPixel(15,4,1);
      matrix.drawPixel(16,4,1);
    }else if(selectedAlarm == 2){//down right arrow edit on off
      matrix.drawPixel(15,0,1);
      matrix.drawPixel(15,1,1);
      matrix.drawPixel(15,2,1);
      matrix.drawPixel(16,1,1);
    }
  }else if(state == 5){//alarm goes off
    if(millis() - alarmBlinkTimer>=500){
      if(alarmState==0){
        noTone(buzzPin);
        for (int i = 0;i<32;i++){
          for(int j=0;j<8;j++){
            matrix.drawPixel(i,j,alarmLogo[j][i]);  
          }
        } 
        alarmState = 1;
      }else if(alarmState==1){
        tone(buzzPin, 415, 500);
        for (int i = 0;i<32;i++){
          for(int j=0;j<8;j++){
            matrix.drawPixel(i,j,!alarmLogo[j][i]);  
          }
        } 
        alarmState = 0;
      }
      alarmBlinkTimer = millis();
    }   
  }
  matrix.write();
}

void addOneTime(){
  int upperLimit=0;
  int lowerLimit=0;
  switch(selectedTime){
    case 3:  upperLimit = 23; lowerLimit = 0;break; //hour
    case 4:  upperLimit = 59; lowerLimit = 0;break; //minute
  }
  if(timeData[selectedTime] < upperLimit){
    timeData[selectedTime]++;
  }else if(timeData[selectedTime] == upperLimit){
    timeData[selectedTime] = lowerLimit;
  }
  drawDisplay();

}

void minusOneTime(){
  int lowerLimit=0;
  int upperLimit=0;
  switch(selectedTime){
    case 3: upperLimit = 23;lowerLimit = 0;break; //hour
    case 4: upperLimit = 59;lowerLimit = 0;break; //minute
  }
  if(timeData[selectedTime] > lowerLimit){
    timeData[selectedTime]--;
  }else if(timeData[selectedTime] == lowerLimit){
    timeData[selectedTime] = upperLimit;
  }
  drawDisplay();
}

void addOneAlarm(){
  int upperLimit=0;
  int lowerLimit=0;
  switch(selectedAlarm){
    case 0:  upperLimit = 23; lowerLimit = 0;break; //hour
    case 1:  upperLimit = 59; lowerLimit = 0;break; //minute
    case 2:  upperLimit = 1; lowerLimit = 0;break; //on/off
  }
  if(alarmData[selectedAlarm] < upperLimit){
    alarmData[selectedAlarm]++;
  }else if(alarmData[selectedAlarm] == upperLimit){
    alarmData[selectedAlarm] = lowerLimit;
  }
  drawDisplay();
}

void minusOneAlarm(){
  int lowerLimit=0;
  int upperLimit=0;
  switch(selectedAlarm){
    case 0: upperLimit = 23;lowerLimit = 0;break; //hour
    case 1: upperLimit = 59;lowerLimit = 0;break; //minute
    case 2:  upperLimit = 1; lowerLimit = 0;break; //on/off
  }
  if(alarmData[selectedAlarm] > lowerLimit){
    alarmData[selectedAlarm]--;
  }else if(alarmData[selectedAlarm] == lowerLimit){
    alarmData[selectedAlarm] = upperLimit;
  }
  drawDisplay();
}

void updateTimeData(){
  //Time t(2013, 9, 22, 01, 38, 50, Time::kTuesday);
  Time t(timeData[0], timeData[1], timeData[2], timeData[3], timeData[4], timeData[5],  timeData[6]);
  rtc.time(t);
}
void updateAlarmData(){
  //update the alarm data to the eeprom  
}

void changeBrightness(){
  photocellReading = analogRead(photocellPin);
  int i = map(photocellReading,1023,0,1,15);
  if(state != 5 and abs(brightness-i)>1){//use temp i to see if light has big change to prevent brightness flicking
    brightness = i;
    matrix.setIntensity(brightness); 
  }else if(state == 5){
    matrix.setIntensity(15);  //screen blink when alarm goes off, max brightness
  }
}

void checkAlarm(){  
  if(timeData[3] == alarmData[0]){//hour match
    if(timeData[4] == alarmData[1]){//minute match
      if(alarmData[2] and state != 3 and millis()-snoozeTimer>60000){//do not go off if user is setting alarm  or user already snoozed the alarm
        state = 5;
      }
    }else if(timeData[4] != alarmData[1] and state == 5){
      state = 0;  
    }
  }
}

void fetchAlarmData(){
  for(int i=0;i<3;i++){
    alarmData[i]= EEPROM.read(i);
  }  
}

void writeAlarmData(){
  for(int i=0;i<3;i++){
    EEPROM.write(i, alarmData[i]);
  }
}


