/*
 *Digital Clock V0.4.0 
 *by Hardi Huang
 *
 *update log:
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
 * tempture sensor = A2;
 * 
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

static unsigned char*  menuArray[8] ={empty_bitmap,setClock_bitmap,setAlarm_bitmap,countDown_bitmap,stopWatch_bitmap,scoreBoard_bitmap,dice_bitmap,empty_bitmap};
int menuSelected =1;

int buzzPin = 9;
int photocellPin = 1;

int btnLeft = 0;
int btnRight = 0;
int btnSet = 0;
int timeData[7] = {2018,9,28,18,34,44,4}; //year,month,date,hour,minute,second,day
int alarmData[3] = {8,0,1};//hour,minute,on or off
unsigned long snoozeTimer = millis();
int state = 0; //0==display mode; 1==menu; 2==alarm goes off; 3==set time; 4==set alarm; 5==count down; 6==stop watch; 7==score board; 8==dice;
int selectedTime = 3; //which one we are editing right now,same order with timeData
int selectedAlarm = 0;
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};
unsigned long syncTimer = millis();
unsigned long editTimer = millis();
unsigned long dotTimer = millis();
bool dotState = 0;
int photocellReading;
int brightness=1;//1-15
unsigned long alarmBlinkTimer = millis();
bool alarmState = 1;
int spacer = 1;  // dots between letters
int width = 5 + spacer; // The font width is 5 pixels + spacer

void setup() {
  matrix.setIntensity(brightness);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);

/*
  matrix.fillScreen(LOW); // show black
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
*/
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);
  Serial.begin(9600); 
  rtc.writeProtect(false);
  rtc.halt(false);
  writeAlarmData();
  fetchAlarmData();
  getTime();
}

void loop() {
  //Serial.println(state);
  changeBrightness();
  checkAlarm();
  getKey();
  if(state == 0){//display time mode
    if(key == "S"){//go to menu
      state = 1; 
      editTimer = millis(); 
    }
    if(millis()-syncTimer>=10000){
      getTime();
      syncTimer = millis();
    }
    
  }else if(state == 1){//menu mode
    if(key == "L"){
      editTimer = millis(); 
      if(menuSelected>1){
        menuSelected--;
        menuAnimation(1);
      }else{
        //menuSelected = 6; 
      }
    }else if(key == "R"){
      editTimer = millis(); 
      if(menuSelected<6){
        menuSelected++;
        menuAnimation(-1);
      }else{
        //menuSelected = 1;  
      }
    }else if(key == "S"){
      editTimer = millis(); 
      state = menuSelected+2;
      menuSelected=1;
    }
  }else if(state == 2){//alarm goes off
    //BUZZ code in the drawDisplay 
    editTimer = millis();
    if(key == "S"){
      noTone(buzzPin);
      digitalWrite(buzzPin, LOW); 
      snoozeTimer = millis();
      matrix.setIntensity(brightness); 
      state = 0;
    }
  }else if(state == 3){//set time mode
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
  }else if(state == 4){//set alarm mode
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
  }else if(state == 5){//countdown mode
    if(key == "S"){
      state = 0;  
    }
  }else if(state == 6){//stopwatch mode
    if(key == "S"){
      state = 0;  
    } 
  }else if(state == 7){//scoreboard mode
    if(key == "S"){
      state = 0;  
    }  
  }else if(state == 8){//dice mode
    if(key == "S"){
      state = 0;  
    }  
  }

  if(state != 0 and millis() - editTimer > 15000){
    state = 0;  
    selectedTime = 3;
    menuSelected=1;
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
  if(state != 2){
    matrix.fillScreen(LOW);
  }
  
  if(state == 0 or state == 3){//display clock mode
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
      matrix.fillRect(15, 1, 2, 2, dotState);
      matrix.fillRect(15, 5, 2, 2, dotState);
    }else if(state == 3){ 
      if(selectedTime == 3){//left arrow edit hour
        matrix.drawRect(16, 3, 1, 3, 1);
        matrix.drawPixel(15,4,1);
      }else if(selectedTime == 4){//right arrow edit minute
        matrix.drawRect(15, 3, 1, 3, 1);
        matrix.drawPixel(16,4,1);
      }
    }
  }else if(state == 1){
    matrix.drawBitmap(1,0,menuArray[menuSelected-1],8,8,1);
    matrix.drawBitmap(12,0,menuArray[menuSelected],8,8,1);
    matrix.drawBitmap(23,0,menuArray[menuSelected+1],8,8,1);
  }else if(state == 4){//set alarm
    String strHrAlarm = String(alarmData[0]); 
    if(alarmData[0]<10){
      strHrAlarm="0"+strHrAlarm;
    }
    String strMinAlarm = String(alarmData[1]);
    if(alarmData[1]<10){
      strMinAlarm="0"+strMinAlarm;
    }
  //draw hours
    matrix.drawChar(1, 0, strHrAlarm.charAt(0),HIGH,LOW, 1);
    matrix.drawChar(8, 0, strHrAlarm.charAt(1),HIGH,LOW, 1);
  //draw minutes
    matrix.drawChar(19, 0, strMinAlarm.charAt(0), HIGH, LOW, 1);
    matrix.drawChar(26, 0, strMinAlarm.charAt(1), HIGH, LOW, 1);
  //draw on off indicator
    if(alarmData[2]==1){
      matrix.drawPixel(31,7,1);
    }else{
      matrix.drawPixel(31,7,0);  
    }
  //draw dot arrow
    if(selectedAlarm == 0){//left arrow edit hour
      matrix.drawRect(16, 3, 1, 3, 1);
      matrix.drawPixel(15,4,1);
    }else if(selectedAlarm == 1){//right arrow edit minute
      matrix.drawRect(15, 3, 1, 3, 1);
      matrix.drawPixel(16,4,1);
    }else if(selectedAlarm == 2){//down right arrow edit on off
      matrix.drawRect(15, 5, 1, 3, 1);
      matrix.drawPixel(16,6,1);
    }
  }else if(state == 2){//alarm goes off
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
  }else if(state == 5){
    centerPrint("test");
  }else if(state == 6){
    centerPrint("test");
  }else if(state == 7){
    centerPrint("test");
  }else if(state == 8){
    centerPrint("test");
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

void checkAlarm(){  
  if(timeData[3] == alarmData[0]){//hour match
    if(timeData[4] == alarmData[1]){//minute match
      if(alarmData[2] and state != 4 and millis()-snoozeTimer>60000){//do not go off if user is setting alarm  or user already snoozed the alarm
        state = 2;
      }
    }else if(timeData[4] != alarmData[1] and state == 2){
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

void scrollMessage(String msg) {
  msg += " "; // add a space at the end
  for ( int i = 0 ; i < width * msg.length() + matrix.width() - 1 - spacer; i++ ) {
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < msg.length() ) {
        matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(20);
  }
  matrix.setCursor(0,0);
}

void centerPrint(String msg) {
  int x = (matrix.width() - (msg.length() * width)) / 2;
  matrix.setCursor(x, 0);
  matrix.print(msg);
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

void stopWatch(){
    
}
