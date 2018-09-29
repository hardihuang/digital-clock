/*
 *Digital Clock V0.2.1 
 *by Hardi Huang - Sep 29 2018
 *
 *update log:
 *  sep/29/2018 13:13 added photocell change brightness function
*/


/* 
max7219 matrix display CONNECTIONS:
 *CLK -> 13
 *CS -> 10
 *DIN -> 11
BUTTON CONNECTIONS:
 *10k pull-up button
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
int alarmData[3] = {8,30,0};//hour,minute,on or off
int state = 0; //0==display mode; 1==menu; 2==set time; 3==set alarm;
int selectedTime = 3; //which one we are editing right now,same order with timeData
int selectedAlarm = 0;
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};
unsigned long editTimer = millis();
unsigned long dotTimer = millis();
bool dotState = 1;
int photocellPin = 1;
int photocellReading;
int brightness;//1-15

void setup() {
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  Serial.begin(9600); 
  rtc.writeProtect(false);
  rtc.halt(false);
  matrix.setIntensity(10);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  
}

void loop() {
  Serial.print("selected time: ");
  Serial.println(selectedTime);
  changeBrightness();
  getKey();
  if(state == 0){//display time mode
    if(key == "S"){//go to menu
      state = 1;  
    }
    getTime();
  }else if(state == 1){{//menu mode
    if(key == "L"){//go to set time mode
      state = 2;
    }else if(key == "R"){//go to set alarm mode
      state = 3;
    }else if(key == "S"){//exit
      state = 0;  
    }
  }
  }else if(state == 2){//set time mode
    if(key == "L"){
      minusOneTime();  
    }else if(key == "R"){
      addOneTime();  
    }else if(key == "S"){
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
    }else if(key == "R"){
      addOneAlarm();  
    }else if(key == "S"){
      if(selectedAlarm<4){
        selectedAlarm++;  
      }else{
        selectedAlarm = 0;
        state = 0;  
        updateAlarmData();
      }
    }
  }
  
  drawDisplay();
  key = "0";
  delay(100);
}

void getKey(){
  int keyArray[] = {5,3,4};//left, right, set
  for(int i = 0; i < 3; i++){
    if(digitalRead(keyArray[i]) == 1){
      if(debounce[i] == 0){
        key = hexaKeys[i];
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
  matrix.fillScreen(LOW);
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
    matrix.drawChar(1, 0, strHrTime.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 0, strHrTime.charAt(1),HIGH,LOW, 1);
  //draw minutes
    matrix.drawChar(19, 0, strMinTime.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 0, strMinTime.charAt(1), HIGH, LOW, 1);
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
  }else if(state == 3){

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
  Serial.println("add one time!");
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
  Serial.println("minus one time!");
}

void addOneAlarm(){
  int upperLimit=0;
  int lowerLimit=0;
  switch(selectedAlarm){
    case 3:  upperLimit = 23; lowerLimit = 0;break; //hour
    case 4:  upperLimit = 59; lowerLimit = 0;break; //minute
  }
  if(alarmData[selectedAlarm] < upperLimit){
    alarmData[selectedAlarm]++;
  }else if(timeData[selectedAlarm] == upperLimit){
    alarmData[selectedAlarm] = lowerLimit;
  }
  drawDisplay();
}

void minusOneAlarm(){
  int lowerLimit=0;
  int upperLimit=0;
  switch(selectedAlarm){
    case 3: upperLimit = 23;lowerLimit = 0;break; //hour
    case 4: upperLimit = 59;lowerLimit = 0;break; //minute
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
  //Serial.println(photocellReading);
  brightness = map(photocellReading,800,0,5,15);
  //Serial.println(brightness);
  matrix.setIntensity(brightness);
}

