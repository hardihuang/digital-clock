/*
 *Digital Clock V0.5.1
 *by Hardi Huang
 *
 *update log:
 *  V0.5.1 Oct/20/2018 18:56 added stopwatch app
 *  V0.5.0 Oct/20/2018 17:00 rebuild the whole system
 *  V0.4.0 Oct/7/2018 23:30 added souch sensor and new perfboard design
 *  V0.3.0 Oct/6/2018 11:30 added menu interface with animation
 *  V0.2.8 Oct/4/2018 18:22 reduced the sram space, ready for adding more functions
 *  V0.2.7 Oct/3/2018 20:29 added opening greeting hello message 
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
 *touch sensor button
 *btnLeft -> D5
 *btnRight -> D3
 *btnSet -> D4
RTC 1302 CLOCK CONNECTIONS:
 * DAT ->D6
 * CLK ->D7
 * RST ->D8
OTHER CONNECTIONS:
 * buzzPin = 9;
 * photocellPin = A1;
 * tiltSwitch sensor = A2;
 * 
*/

//include
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
#include <DS1302.h>
#include <EEPROM.h>

//pin define
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
namespace {
  const int kCePin   = 8;  //RST
  const int kIoPin   = 6;  //DAT
  const int kSclkPin = 7;  //CLK
  DS1302 rtc(kCePin, kIoPin, kSclkPin);
}
int buzzPin = 9;
int photocellPin = 1;
int btnLeft = 5;
int btnRight = 3;
int btnSet = 4;
int tiltSwitch = A2;

//bitmap
static const unsigned char empty_bitmap[] =
{ B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

static const unsigned char setClock_bitmap[] =
{ B00111100,
  B01010010,
  B10010001,
  B10010001,
  B10001001,
  B10000101,
  B01000010,
  B00111100 
};

static const unsigned char setAlarm_bitmap[] =
{ B00011000,
  B00111100,
  B01100110,
  B01000010,
  B01000010,
  B11000011,
  B11111111,
  B00011000
};
static const unsigned char countDown_bitmap[] =
{ B00000000,
  B01111100,
  B01000100,
  B00101000,
  B00010000,
  B00101000,
  B01000100,
  B01111100
};
static const unsigned char stopWatch_bitmap[] =
{
  B00011100,
  B01001000,
  B00111110,
  B01000001,
  B01001001,
  B01001101,
  B01000001,
  B00111110
};
static const unsigned char scoreBoard_bitmap[] =
{
  B00000000,
  B00000000,
  B10100110,
  B10101000,
  B10100100,
  B10100010,
  B01001100,
  B00000000
};
static const unsigned char dice_bitmap[] =
{
  B00000000,
  B01111100,
  B10000010,
  B10101010,
  B10010010,
  B10101010,
  B10000010,
  B01111100
};

//timer
unsigned long stopWatchTimer = millis();
unsigned long snoozeTimer = millis();
unsigned long syncTimer = millis();
unsigned long editTimer = millis();
unsigned long dotTimer = millis();
unsigned long alarmBlinkTimer = millis();

//state
int state = 0; //0==display mode; 1==menu; 2==alarm goes off; 3==set time; 4==set alarm; 5==count down; 6==stop watch; 7==score board; 8==dice;
int menuSelected =1;
int selected =0; //which one we are editing right now,0->hour, 1->minute, 2->second
bool dotState = 0;
bool alarmState = 1;
int rotationState = 0;//0->level;1->tilt left

//key
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};

//data
static unsigned char*  menuArray[8] ={empty_bitmap,setClock_bitmap,setAlarm_bitmap,countDown_bitmap,stopWatch_bitmap,scoreBoard_bitmap,dice_bitmap,empty_bitmap};
int timeData[3] = {8,0,0}; //hour,minute,second
int alarmData[4] = {8,0,0,1};//hour,minute,second, on or off
int stopWatchData[4] = {0,0,0,0};
int countDownData[4] = {0,0,0,0};

//others
int photocellReading;
int brightness=1;//1-15

void setup() {
  Serial.begin(9600);
  displaySetup();
  pinSetup();
  rtcSetup();
  fetchAlarmData();
  getTime();
  //greating();
}

void loop() {
  changeBrightness();
  checkAlarm();
  getKey();
  //control flow
  switch(state){
    case 0://display mode clock face
      if(key == "S"){state = 1;}
      if(millis()-syncTimer>=10000){
        getTime();
        syncTimer = millis();  
      }
      clockFace();
    break;
    case 1://menu mode
      menu();
      if(key == "L"){
        if(menuSelected>1){
          menuSelected--;
          menuAnimation(1);
        }else{//exit to clock face
          menuSelected = 1;
          state = 0;
        }
      }else if(key == "R"){
        if(menuSelected<6){
          menuSelected++;
          menuAnimation(-1);
        }else{
          //menuSelected = 1;  
        }
      }else if(key == "S"){
        state = menuSelected+2;
        menuSelected=1;
      }
    break;
    case 2://alarm goes off
      alarmOn();
      getTime();//update time data
      if(key == "S" || key == "L" || key == "R"){
        noTone(buzzPin);
        digitalWrite(buzzPin, LOW); 
        snoozeTimer = millis();
        matrix.setIntensity(brightness); 
        state = 0;
      }
    break;
    case 3://set time mode
      if(key == "L"){
        addMinusOne(0,timeData); 
      }else if(key == "R"){
        addMinusOne(1,timeData); 
      }else if(key == "S"){
        if(selected<1){
          selected++;  
        }else{
          selected= 0;
          state = 0;  
          timeData[2]=0;//reset second
          updateTimeData();
        }
      }
      setTime();
    break;
    case 4://set alarm mode
      if(key == "L"){
        addMinusOne(0,alarmData);  
        writeAlarmData();
      }else if(key == "R"){
        addMinusOne(1,alarmData); 
        writeAlarmData();
      }else if(key == "S"){
        if(selected<1){
          selected++;
        }else if(selected == 1){
          selected = 3;
        }else{
          selected = 0;
          state = 0;  
        }
        writeAlarmData();
      }
      setAlarm();
    break;
    case 5://count down mode
      countDown();
      if(key == "S"){state = 0;}
    break;
    case 6://stop watch mode
      stopWatch();
      if(key == "S"){
        if(stopWatchData[3] == 0){
          stopWatchTimer = millis();
          stopWatchData[3]=1;//start stopWatch
        }else{
          stopWatchData[3]=0;//stop stopWatch
        }
      }else if(key == "L"){//exit
        stopWatchData[0]=0;
        stopWatchData[1]=0;
        stopWatchData[2]=0;
        stopWatchData[3]=0;
        state = 0;
      }else if(key == "R"){//reset
        stopWatchData[0]=0;
        stopWatchData[1]=0;
        stopWatchData[2]=0;
        stopWatchData[3]=0;
      }
    break;
    case 7://score board mode
      scoreBoard();
      if(key == "S"){state = 0;}
    break;
    case 8://dice mode
      dice();
      if(key == "S"){state = 0;}
    break;

    default:
    break;
  }//end switch


  key = "0";
  delay(100);
}

/****
 * setup functions
****/
void displaySetup(){
  matrix.setIntensity(brightness);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  matrix.fillScreen(LOW); // show black
  matrix.write();
}

void pinSetup(){
  pinMode(btnLeft,INPUT);
  pinMode(btnRight,INPUT);
  pinMode(btnSet,INPUT);
  pinMode(tiltSwitch, INPUT);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);  
}

void rtcSetup(){
  rtc.writeProtect(false);
  rtc.halt(false);  
}

/****
 * loop routine functions
****/
void checkAlarm(){  
  if(timeData[0] == alarmData[0]){//hour match
    if(timeData[1] == alarmData[1]){//minute match
      if(alarmData[3] and state != 4 and millis()-snoozeTimer>60000){//do not go off if user is setting alarm  or user already snoozed the alarm
        state = 2;
      }
    }else if(timeData[1] != alarmData[1] and state == 2){
      state = 0;  
    }
  }
}

void changeBrightness(){
  photocellReading = analogRead(photocellPin);
  int i ;
  if(photocellReading<=750){
    i = map(photocellReading,750,0,0,15);
  }else{
    i=0;
  }
  if(state != 2 and abs(brightness-i)>1){//use temp i to see if light has big change to prevent brightness flicking
    brightness = i;
    matrix.setIntensity(brightness); 
  }else if(state == 2){
    matrix.setIntensity(15);  //screen blink when alarm goes off, max brightness
  }
}

void getKey(){
  int keyArray[] = {btnLeft,btnRight,btnSet};//left, right, set
  for(int i = 0; i < 3; i++){
    if(digitalRead(keyArray[i]) == 1){
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

/****
 * utility
****/

void greating(){
  matrix.setCursor(1, 1);
  centerPrint("hello");
  
  tone(buzzPin, 415, 500);
  tone(buzzPin, 415, 500);
  delay(500*1.3);
  tone(buzzPin, 466, 500);
  delay(500*1.3);
  tone(buzzPin, 370, 1000);
  delay(1000*1.3);
  noTone(buzzPin);

  matrix.fillScreen(LOW); // show black
  matrix.write();
  delay(500);
  scrollMessage("Please Enjoy!");
  matrix.fillScreen(LOW); // show black
  delay(1000);
}
void checkRotation(){
    if(digitalRead(tiltSwitch)){
      rotationState = 0;
      
      numberOfHorizontalDisplays = 4;
      numberOfVerticalDisplays = 1;
      matrix.setRotation(0, 1);
      matrix.setRotation(1, 1);
      matrix.setRotation(2, 1);
      matrix.setRotation(3, 1);
    }else{ 
      rotationState = 1;//tile left
      //numberOfHorizontalDisplays = 1;
      //numberOfVerticalDisplays = 4;
      //matrix.setRotation(0, 2);
      //matrix.setRotation(1, 2);
      //matrix.setRotation(2, 2);
      //matrix.setRotation(3, 2);
    }   
    //Serial.println(rotationState);  
}

void centerPrint(String msg) {
  matrix.fillScreen(LOW);
  int x = (matrix.width() - (msg.length() * 6)) / 2;
  matrix.setCursor(x, 0);
  matrix.print(msg);
  matrix.write();
}

void scrollMessage(String msg) {
  msg += " "; // add a space at the end
  for ( int i = 0 ; i < 6 * msg.length() + matrix.width() - 1 - 1; i++ ) {
    int letter = i / 6;
    int x = (matrix.width() - 1) - i % 6;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + 6 - 1 >= 0 && letter >= 0 ) {
      if ( letter < msg.length() ) {
        matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= 6;
    }
    matrix.write(); // Send bitmap to display
    delay(20);
  }
  matrix.setCursor(0,0);
}

void writeAlarmData(){
  for(int i=0;i<3;i++){
    EEPROM.write(i, alarmData[i]);
  }
}
void fetchAlarmData(){
  for(int i=0;i<3;i++){
    alarmData[i]= EEPROM.read(i);
  }  
}
void getTime(){
  Time t = rtc.time();
  timeData[0] = t.hr;
  timeData[1] = t.min;
  timeData[2] = t.sec;
}
void updateTimeData(){
  //Time t(2013, 9, 22, 01, 38, 50, Time::kTuesday);
  Time t(2000, 1, 1, timeData[0], timeData[1], timeData[2], 1);
  rtc.time(t);
}
//operation: 0->minus; 1->add;  
void addMinusOne(int operation, int arr[]){
  int upperLimit;
  int lowerLimit;
  switch(selected){
    case 0://hour
      upperLimit = 23;
      lowerLimit = 0;
      break;
    case 1://minute
      upperLimit = 59;
      lowerLimit = 0;
      break;
    case 2://second
      upperLimit = 59;
      lowerLimit = 0; 
      break;
    case 3://on or off
      upperLimit = 1;
      lowerLimit = 0;
      break;
  }
  switch(operation){
    case 0://minuse one
      if(arr[selected]>lowerLimit){
        arr[selected]--;  
      }else if(arr[selected] == lowerLimit){
        arr[selected] = upperLimit;  
      }
    break;
    case 1:
      if(arr[selected]<upperLimit){
        arr[selected]++;  
      }else if(arr[selected] == upperLimit){
        arr[selected] = lowerLimit;  
      }
    break;
  }
}

String formateDigit(int digitTemp){
    String strTemp = String(digitTemp);
    if(digitTemp<10){
      strTemp = "0"+strTemp;  
    }
    return strTemp;
}

/****
 * drawDisplay functions fordifferent interfaces
****/

void clockFace(){
  String strHr = formateDigit(timeData[0]);
  String strMin = formateDigit(timeData[1]);
  String strSec = formateDigit(timeData[2]);
  
  matrix.fillScreen(LOW);
  //draw hours
    matrix.drawChar(1, 0, strHr.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 0, strHr.charAt(1),HIGH,LOW, 1);
    //draw minutes
    matrix.drawChar(19, 0, strMin.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 0, strMin.charAt(1), HIGH, LOW, 1);
  //draw dots
    if(millis()-dotTimer>=500  ){
      if(dotState == 1){
        dotState = 0;
      }else if(dotState == 0 and state == 0){
        dotState = 1;
      }
      dotTimer = millis();  
    }
    matrix.fillRect(15, 1, 2, 2, dotState);
    matrix.fillRect(15, 5, 2, 2, dotState);
    matrix.write();
}

void menu(){
    matrix.fillScreen(LOW);
    matrix.drawBitmap(1,0,menuArray[menuSelected-1],8,8,1);
    matrix.drawBitmap(12,0,menuArray[menuSelected],8,8,1);
    matrix.drawBitmap(23,0,menuArray[menuSelected+1],8,8,1);
    matrix.write();  
}
void menuAnimation(int dir){
  int q=0;
  if(dir==1){//left pressed, animation go right
    q=-1;
  }else if(dir == -1){//right pressed, animation go left
    q=-3;
  }
  for(int i=0; i<12;i++){
    matrix.fillScreen(LOW); 
    matrix.drawBitmap(-10+i*dir,0,menuArray[menuSelected+q],8,8,1);
    matrix.drawBitmap(1+i*dir,0,menuArray[menuSelected+q+1],8,8,1);
    matrix.drawBitmap(12+i*dir,0,menuArray[menuSelected+q+2],8,8,1);
    matrix.drawBitmap(23+i*dir,0,menuArray[menuSelected+q+3],8,8,1);
    matrix.drawBitmap(34+i*dir,0,menuArray[menuSelected+q+4],8,8,1);
    matrix.write();
    delay(20);
  }
}
void setTime(){
    matrix.fillScreen(LOW); 
    String strHr = formateDigit(timeData[0]);
    String strMin = formateDigit(timeData[1]);
    String strSec = formateDigit(timeData[2]);
    //draw hours
    matrix.drawChar(1, 0, strHr.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 0, strHr.charAt(1),HIGH,LOW, 1);
    //draw minutes
    matrix.drawChar(19, 0, strMin.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 0, strMin.charAt(1), HIGH, LOW, 1);
  
   if(selected == 0){//left arrow edit hour
      matrix.drawRect(16, 3, 1, 3, 1);
      matrix.drawPixel(15,4,1);
    }else if(selected == 1){//right arrow edit minute
      matrix.drawRect(15, 3, 1, 3, 1);
      matrix.drawPixel(16,4,1);
    }
    matrix.write();
}
void setAlarm(){
    matrix.fillScreen(LOW); 
    String strHr = formateDigit(alarmData[0]);
    String strMin = formateDigit(alarmData[1]);

  //draw hours
    matrix.drawChar(1, 0, strHr.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 0, strHr.charAt(1),HIGH,LOW, 1);
  //draw minutes
    matrix.drawChar(19, 0, strMin.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 0, strMin.charAt(1), HIGH, LOW, 1);
  //draw on off indicator
    if(alarmData[3]==1){
      matrix.drawPixel(31,7,1);
    }else{
      matrix.drawPixel(31,7,0);  
    }
  //draw dot arrow
    if(selected == 0){//left arrow edit hour
      matrix.drawRect(16, 3, 1, 3, 1);
      matrix.drawPixel(15,4,1);
    }else if(selected == 1){//right arrow edit minute
      matrix.drawRect(15, 3, 1, 3, 1);
      matrix.drawPixel(16,4,1);
    }else if(selected == 3){//down right arrow edit on off
      matrix.drawRect(15, 5, 1, 3, 1);
      matrix.drawPixel(16,6,1);
    }
    matrix.write();
}

void alarmOn(){
    if(millis() - alarmBlinkTimer>=500){
      if(alarmState==0){
        noTone(buzzPin);
        matrix.fillScreen(LOW);
        matrix.drawBitmap(12, 0,  menuArray[2], 8, 8, 1);
        alarmState = 1;
      }else if(alarmState==1){
        tone(buzzPin, 415, 500);
        matrix.fillScreen(HIGH);
        matrix.drawBitmap(12, 0,   menuArray[2], 8, 8, 0);
        alarmState = 0;
      }
      alarmBlinkTimer = millis();
    }  
    matrix.write(); 
}
void countDown(){centerPrint("C D");}
void stopWatch(){
    matrix.fillScreen(LOW); 
    String strMin = formateDigit(stopWatchData[1]);
    String strSec = formateDigit(stopWatchData[2]);
  //draw minutes 0-99
    matrix.drawChar(1, 0, strMin.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 0, strMin.charAt(1),HIGH,LOW, 1);
  //draw seconds 0-59
    matrix.drawChar(19, 0, strSec.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 0, strSec.charAt(1), HIGH, LOW, 1);
  //draw dots
    matrix.fillRect(15, 1, 2, 2, 1);
    matrix.fillRect(15, 5, 2, 2, 1);
    if(stopWatchData[3]){
      if( millis() - stopWatchTimer>=1000){ //update time
        if(stopWatchData[2]<59){
          stopWatchData[2]++;  
        }else{
          if(stopWatchData[1]<90){
            stopWatchData[1]++;
            stopWatchData[2]=0;  
          }else{//reset and stop counting
            stopWatchData[0]=0;
            stopWatchData[1]=0;
            stopWatchData[2]=0;
            stopWatchData[3]=0;
          }
        }
        stopWatchTimer = millis();
      }
    }
    matrix.write();
}
void scoreBoard(){centerPrint("S B");}
void dice(){centerPrint("D C");}


