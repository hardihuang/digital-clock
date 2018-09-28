#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <DS1302.h>

int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

namespace {
  const int kCePin   = 8;  //RST
  const int kIoPin   = 6;  //DAT
  const int kSclkPin = 7;  //CLK
  DS1302 rtc(kCePin, kIoPin, kSclkPin);
}//namespace


int btnLeft = 0;
int btnRight = 0;
int btnSet = 0;

int timeData[7] = {2018,9,28,18,34,44,4}; //year,month,date,hour,minute,second,day
int state = 0; //0==display mode; 1==set time mode; 2==set alarm mode;
int selected = 0; //which one we are editing right now,same order with timeData
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};
unsigned long editTimer = millis();

void setup() {
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  Serial.begin(9600); 
  rtc.writeProtect(false);
  rtc.halt(false);
  
  matrix.setIntensity(5);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  
}

void loop() {
  Serial.println("get in the loop!");
  Serial.println(state);
  Serial.println(selected);
  getKey();
  Serial.println(key);
  
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
  delay(100);
  
}

void getKey(){
  int keyArray[] = {5,3,4};//left, right, set
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

  Serial.println(tempYr+" "+tempMon+"/"+tempDate+" "+tempHr+" "+tempMin+" "+tempSec+" "+tempDay);
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



