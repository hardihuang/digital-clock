/*
8*8 matrix CONNECTIONS:
* rst->8;dat->6;clk->7
BUTTON CONNECTIONS:
 *10k pull-up button
 *btnLeft -> D4
 *btnRight -> D3
 *btnSet -> D2
RTC 1302 CLOCK CONNECTIONS:
 * DAT ->D6
 * CLK ->D7
 * RST ->D8tr
*/
#include <stdio.h>
#include <DS1302.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
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

int timeData[7] = {2018,9,27,22,13,5,3}; //year,month,date,hour,minute,second,day
int state = 0; //0==display mode; 1==set time mode; 2==set alarm mode;
int selected = 0; //which one we are editing right now,same order with timeData
int debounce[3];
int keyArr[3] = {3,4,5};//left, right, set
char key='0';
char hexaKeys[]={'L','R','S'};
unsigned long editTimer = millis();
unsigned long gtimer = millis();
unsigned long sectimer = millis();


void setup() {
  matrix.setIntensity(5);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  Serial.begin(9600); 
}


void loop() {
  Serial.print("state: ");
  Serial.println(state);
  Serial.print("selected: ");
  Serial.println(selected);
  //getKey();
  Serial.print("key: ");
  Serial.println(key);
  Serial.println("");
  if(state == 0){
    if(millis()-gtimer>5000){
      Serial.println("sync real time");
      getTime();
      gtimer = millis();
    }else if(millis()-sectimer >= 1000){
      Serial.println("fake time + 1");
      timeData[5]++;
      sectimer = millis();
    }
    drawDisplay();
    if(key == 'S'){
      state = 1;
      selected = 0;
      editTimer = millis();
      key = '0';
    }
  }else if(state == 1 and key!='0'){
    Serial.println("state 1");
    if(key == 'R'){
      editTimer = millis();
      addOne();
    }else if(key == 'L'){
      editTimer = millis();
      minusOne();  
    }else if(key == 'S'){
      editTimer = millis();
      if(selected<6){
        selected++;
      }else{
        selected = 0;
        state = 0;
        updateTime();
      }
    }
    key = '0';
  }
  if(state != 0 and millis() - editTimer > 15000){
    state = 0;  
    selected = 0;
  }
  delay(100);
  
}
void getKey(){
  for(int i = 0; i < 3; i++){ 
    if(digitalRead(keyArr[i]) == 1){
      if(debounce[i] == 0){
        key = hexaKeys[i];
        debounce[i] = 2;  
      }else{
        debounce[i] -= 1;  
      }
    }else{
      debounce[i] = 0;
    }
  }
}

void getTime(){
  
  Time tIn = rtc.time();
  
  timeData[0] = tIn.yr;
  timeData[1] = tIn.mon;
  timeData[2] = tIn.date;
  timeData[3] = tIn.hr;
  timeData[4] = tIn.min;
  timeData[5] = tIn.sec;
  timeData[6] = tIn.day;
  
  for(int i=0;i<7;i++){
    Serial.println(timeData[i]);
  }
}

void drawDisplay(){
  String tempStr;
  char tempHr[2];
  char tempMin[2];
  char tempSec[2];

  tempStr="";
  tempStr = String(timeData[4]);
  if(timeData[4]<10){
    tempStr="0"+tempStr;
  }
  tempStr.toCharArray(tempMin,3);
  tempStr="";
  
  tempStr = String(timeData[3]);  
  if(timeData[3]<10){
    tempStr="0"+tempStr;
  }
  tempStr.toCharArray(tempHr,3);
  tempStr="";
  
  tempStr = String(timeData[5]); 
  if(timeData[5]<10){
    tempStr="0"+tempStr;
  }
  tempStr.toCharArray(tempSec,3);
  tempStr="";
/*
  Serial.println(timeData[0]);
  Serial.println(timeData[1]);
  Serial.println(timeData[2]);
  Serial.println(timeData[3]);
  Serial.println(tempHr[0]);
  Serial.println(tempHr[1]);
  Serial.println(timeData[4]);
  Serial.println(tempMin[0]);
  Serial.println(tempMin[1]);
  Serial.println(timeData[5]);
  Serial.println(tempSec[0]);
  Serial.println(tempSec[1]);
  Serial.println(timeData[6]);
  */
  matrix.fillScreen(LOW);
  //hours
  matrix.drawChar(1, 0, tempMin[0],HIGH,LOW, 1);
  matrix.drawChar(8, 0, tempMin[1],HIGH,LOW, 1);
  //dots
  matrix.drawPixel(15,1,HIGH);
  matrix.drawPixel(15,2,HIGH);
  matrix.drawPixel(16,1,HIGH);
  matrix.drawPixel(16,2,HIGH);
  matrix.drawPixel(15,5,HIGH);
  matrix.drawPixel(15,6,HIGH);
  matrix.drawPixel(16,5,HIGH);
  matrix.drawPixel(16,6,HIGH);
  //minutes
  matrix.drawChar(19, 0, tempSec[0], HIGH, LOW, 1);
  matrix.drawChar(26, 0, tempSec[1], HIGH, LOW, 1);
  matrix.write();
}

void addOne(){
  Serial.println("added One");
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
  Serial.println("minused One");
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

void updateTime(){
  rtc.writeProtect(false);
  rtc.halt(false);
  //Serial.println("updated");
  //Time t(2013, 9, 22, 01, 38, 50, Time::kTuesday);
  Time tOut(timeData[0], timeData[1], timeData[2], timeData[3], timeData[4], timeData[5],  timeData[6]);
  rtc.time(tOut);
}

