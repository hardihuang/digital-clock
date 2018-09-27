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
 * RST ->D8
*/

#include <DS1302.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int timer;

namespace {
  const int kCePin   = 8;  //RST
  const int kIoPin   = 6;  //DAT
  const int kSclkPin = 7;  //CLK
  DS1302 rtc(kCePin, kIoPin, kSclkPin);
}//namespace

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
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  
  pinMode(9, INPUT);
  pinMode(13, INPUT);
  pinMode(10, INPUT);
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
  int keyArray[] = {4,3,2};//left, right, set
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
  char tempHrOne = timeData[3]%10;
  char tempHrTen = (timeData[3]/10)%10;
  char tempMinOne = timeData[4]%10;
  char tempMinTen = (timeData[4]/10)%10;
  
  //formate the single digit data
  //if(timeData[1]<10){tempMon = "0"+tempMon;}
  //if(timeData[2]<10){tempDate = "0"+tempDate;}
  //if(timeData[3]<10){tempHr = "0"+tempHr;}
  //if(timeData[4]<10){tempMin = "0"+tempMin;}
  //if(timeData[5]<10){tempSec = "0"+tempSec;}
  //String fullDate = " "+tempYr+" "+tempMon+"/"+tempDate+" "+tempDay;
  //String fullTime = "    "+tempHr+":"+tempMin+":"+tempSec+"    ";

  //Serial.println(tempYr+" "+tempMon+"/"+tempDate+" "+tempHr+" "+tempMin+" "+tempSec+" "+tempDay);
  
  matrix.fillScreen(LOW);
  matrix.drawChar(1, 0, tempHrTen, HIGH, LOW, 1);
  matrix.drawChar(8, 0, tempHrOne, HIGH, LOW, 1);
  if(timer){
    matrix.drawPixel(15,1,HIGH);
    matrix.drawPixel(15,2,HIGH);
    matrix.drawPixel(16,1,HIGH);
    matrix.drawPixel(16,2,HIGH);
    matrix.drawPixel(15,5,HIGH);
    matrix.drawPixel(15,6,HIGH);
    matrix.drawPixel(16,5,HIGH);
    matrix.drawPixel(16,6,HIGH);
    timer = 0;
  }else{
    matrix.drawPixel(15,1,LOW);
    matrix.drawPixel(15,2,LOW);
    matrix.drawPixel(16,1,LOW);
    matrix.drawPixel(16,2,LOW);
    matrix.drawPixel(15,5,LOW);
    matrix.drawPixel(15,6,LOW);
    matrix.drawPixel(16,5,LOW);
    matrix.drawPixel(16,6,LOW);
    timer = 1;
  }
  matrix.drawChar(19, 0, tempMinTen, HIGH, LOW, 1);
  matrix.drawChar(26, 0, tempMinOne, HIGH, LOW, 1);
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



