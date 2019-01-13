/*
LCD CONNECTIONS:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
BUTTON CONNECTIONS:
 *10k pull-up button
 *btnLeft -> D9
 *btnRight -> D13
 *btnSet -> D10
RTC 1302 CLOCK CONNECTIONS:
 * DAT ->D6
 * CLK ->D7
 * RST ->D8
*/
#include <LiquidCrystal.h>
#include <stdio.h>
#include <DS1302.h>

namespace {
  const int kCePin   = 8;  //RST
  const int kIoPin   = 6;  //DAT
  const int kSclkPin = 7;  //CLK
  DS1302 rtc(kCePin, kIoPin, kSclkPin);
}//namespace

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int btnLeft = 0;
int btnRight = 0;
int btnSet = 0;

int timeData[7] = {2000,1,1,0,0,0,1}; //year,month,date,hour,minute,second,day
int state = 0; //0==display mode; 1==set time mode; 2==set alarm mode;
int selected = 0; //which one we are editing right now,same order with timeData
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};
unsigned long editTimer = millis();

void setup() {
  pinMode(9, INPUT);
  pinMode(13, INPUT);
  pinMode(10, INPUT);
  lcd.begin(16, 2);
  Serial.begin(9600); 
  rtc.writeProtect(false);
  rtc.halt(false);
}

void loop() {
  //Serial.println(state);
  //Serial.println(selected);
  getKey();
  //Serial.println(key);
  
  if(state == 0){
    getTime();
    drawDisplay();
    if(key == "S"){
      state = 1;
      selected = 0;
      editTimer = millis();
      btnSet = 0;
      key = "0";
    }
  }
  if(state == 1 and key!= "0"){
    if(key == "R"){
      editTimer = millis();
      addOne();
      key = "0";
    }else if(key == "L"){
      editTimer = millis();
      minusOne();  
      key = "0";
    }else if(key == "S"){
      key = "0";
      editTimer = millis();
      if(selected<6){
        selected++;
      }else{
        selected = 0;
        state = 0;
        updateTimeData();
      }
    }
  }
  if(state != 0 and millis() - editTimer > 15000){
    state = 0;  
    selected = 0;
  }
  delay(50);
  
}

void getKey(){
  int keyArray[] = {9,13,10};//left, right, set
  for(int i = 0; i < 3; i++){
    if(digitalRead(keyArray[i]) == 1){
      if(debounce[i] == 0){
        key = hexaKeys[i];
        debounce[i] = 10;  
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
  String tempYr = String(timeData[0]);
  String tempMon = String(timeData[1]);
  String tempDate = String(timeData[2]);
  String tempHr = String(timeData[3]);
  String tempMin = String(timeData[4]);
  String tempSec = String(timeData[5]);
  String tempDay = String(timeData[6]);
  

  switch (timeData[6]) {
    case 1: tempDay = "Sun"; break;
    case 2: tempDay = "Mon"; break;
    case 3: tempDay = "Tue"; break;
    case 4: tempDay = "Wed"; break;
    case 5: tempDay = "Thu"; break;
    case 6: tempDay = "Fri"; break;
    case 7: tempDay = "Sat"; break;
    default: tempDay = "unknown";
  }
  
  //formate the single digit data
  if(timeData[1]<10){tempMon = "0"+tempMon;}
  if(timeData[2]<10){tempDate = "0"+tempDate;}
  if(timeData[3]<10){tempHr = "0"+tempHr;}
  if(timeData[4]<10){tempMin = "0"+tempMin;}
  if(timeData[5]<10){tempSec = "0"+tempSec;}
  String fullDate = " "+tempYr+" "+tempMon+"/"+tempDate+" "+tempDay;
  String fullTime = "    "+tempHr+":"+tempMin+":"+tempSec+"    ";
  
  //print to the screen
  lcd.setCursor(0,0);
  lcd.print(fullDate);
  lcd.setCursor(0,1);
  lcd.print(fullTime);
  //Serial.println(tempYr+" "+tempMon+"/"+tempDate+" "+tempHr+" "+tempMin+" "+tempSec+" "+tempDay);
}

void addOne(){
  //Serial.println("added One");
  int upperLimit=0;
  int lowerLimit=0;
  switch(selected){
    case 0:  upperLimit = 2020; lowerLimit = 2000;  break;  //year
    case 1:  upperLimit = 12; lowerLimit = 1;break; //month
    case 2:  upperLimit = 31; lowerLimit = 1;break; //date
    case 3:  upperLimit = 23; lowerLimit = 0;break; //hour
    case 4:  upperLimit = 59; lowerLimit = 0;break; //minute
    case 5:  upperLimit = 59; lowerLimit = 0;break; // second
    case 6:  upperLimit = 7; lowerLimit = 1;break; // day
  }
  if(timeData[selected] < upperLimit){
    timeData[selected]++;
  }else if(timeData[selected] == upperLimit){
    timeData[selected] = lowerLimit;
  }
  drawDisplay();
}

void minusOne(){
  //Serial.println("minused One");
  int lowerLimit=0;
  int upperLimit=0;
  switch(selected){
    case 0: upperLimit = 2020;lowerLimit = 2000; break;  //year
    case 1: upperLimit = 12;lowerLimit = 1;break; //month
    case 2: upperLimit = 31;lowerLimit = 1;break; //date
    case 3: upperLimit = 23;lowerLimit = 0;break; //hour
    case 4: upperLimit = 59;lowerLimit = 0;break; //minute
    case 5: upperLimit = 59;lowerLimit = 0;break; // second
    case 6: upperLimit = 7;lowerLimit = 1;break; // day
  }
  if(timeData[selected] > lowerLimit){
    timeData[selected]--;
  }else if(timeData[selected] == lowerLimit){
    timeData[selected] = upperLimit;
  }
  drawDisplay();
}

void updateTimeData(){
  //Serial.println("updated");
  //Time t(2013, 9, 22, 01, 38, 50, Time::kTuesday);
  Time t(timeData[0], timeData[1], timeData[2], timeData[3], timeData[4], timeData[5],  timeData[6]);
  rtc.time(t);
}


