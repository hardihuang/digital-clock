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
unsigned long dotTimer = millis();
bool dotFlag = 0;

void setup() {
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  Serial.begin(9600); 
  rtc.writeProtect(false);
  rtc.halt(false);
  
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  
}

void loop() {
  getKey();
  /*
  Serial.print("state: ");
  Serial.println(state);
  Serial.print("selected: ");
  Serial.println(selected);
  Serial.print("key: ");
  Serial.println(key);
  */
  if(state == 0){
    getTime();
    drawDisplay();
    if(key == "S"){
      state = 1;
      selected = 3;
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
      if(selected<4){
        selected++;
      }else{
        selected = 0;
        state = 0;
        timeData[5]=0;//reset second
        updateTimeData();
      }
    }
  }
  if(state != 0 and millis() - editTimer > 15000){
    state = 0;  
    selected = 0;
  }
  delay(500);
  
}

void getKey(){
  int keyArray[] = {5,3,4};//left, right, set
  for(int i = 0; i < 3; i++){
    if(digitalRead(keyArray[i]) == 1){
      if(debounce[i] == 0){
        key = hexaKeys[i];
        debounce[i] = 1;  
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
  /*
  for(int i=0;i<7;i++){
    Serial.print(timeData[i]);
    Serial.print(" ");
  }
  Serial.println("");
  */
}

void drawDisplay(){
  String strHr = String(timeData[3]); 
  if(timeData[3]<10){
    strHr="0"+strHr;
  }

  String strMin = String(timeData[4]);
  if(timeData[4]<10){
    strMin="0"+strMin;
  }

  String strSec = String(timeData[5]); 
  if(timeData[5]<10){
    strSec="0"+strSec;
  }
  /*
  Serial.println(timeData[3]);
  Serial.println(strHr.charAt(0));
  Serial.println(strHr.charAt(1));
  Serial.println(timeData[4]);
  Serial.println(strMin.charAt(0));
  Serial.println(strMin.charAt(1));
  Serial.println(timeData[5]);
  Serial.println(strSec.charAt(0));
  Serial.println(strSec.charAt(1));
  */
  matrix.fillScreen(LOW);
  //hours
  matrix.drawChar(1, 0, strHr.charAt(0),HIGH,LOW, 1);
  matrix.drawChar(8, 0, strHr.charAt(1),HIGH,LOW, 1);

  //dots
  if(dotFlag == 0 and state == 0){//dot on
    matrix.drawPixel(15,1,HIGH);
    matrix.drawPixel(15,2,HIGH);
    matrix.drawPixel(16,1,HIGH);
    matrix.drawPixel(16,2,HIGH);
    matrix.drawPixel(15,5,HIGH);
    matrix.drawPixel(15,6,HIGH);
    matrix.drawPixel(16,5,HIGH);
    matrix.drawPixel(16,6,HIGH);
    dotFlag = 1;
  }else{ //dot off
    matrix.drawPixel(15,1,0);
    matrix.drawPixel(15,2,0);
    matrix.drawPixel(16,1,0);
    matrix.drawPixel(16,2,0);
    matrix.drawPixel(15,5,0);
    matrix.drawPixel(15,6,0);
    matrix.drawPixel(16,5,0);
    matrix.drawPixel(16,6,0);
    dotFlag = 0;
  }
  
  //minutes
  matrix.drawChar(19, 0, strMin.charAt(0), HIGH, LOW, 1);
  matrix.drawChar(26, 0, strMin.charAt(1), HIGH, LOW, 1);
  matrix.write();

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



