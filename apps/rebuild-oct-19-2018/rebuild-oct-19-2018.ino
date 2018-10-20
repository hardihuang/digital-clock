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
static unsigned char*  menuArray[8] ={empty_bitmap,setClock_bitmap,setAlarm_bitmap,countDown_bitmap,stopWatch_bitmap,scoreBoard_bitmap,dice_bitmap,empty_bitmap};
int menuSelected =1;
int selectedTime = 3; //which one we are editing right now,same order with timeData
int selectedAlarm = 0;
bool dotState = 0;
bool alarmState = 1;
int rotationState = 0;//0->level;1->tilt left

//key
int debounce[3];
String key="0";
char hexaKeys[]={'L','R','S'};

//data
int timeData[7] = {2018,9,28,18,34,44,4}; //year,month,date,hour,minute,second,day
int alarmData[3] = {8,0,1};//hour,minute,on or off
int stopWatchData[3] = {0,0,0};//minute, second, on or off

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
  greating();
  delay(1000);
}

void loop() {
  
  centerPrint("Hardi");
  matrix.fillScreen(LOW);
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
  timeData[0] = t.yr;
  timeData[1] = t.mon;
  timeData[2] = t.date;
  timeData[3] = t.hr;
  timeData[4] = t.min;
  timeData[5] = t.sec;
  timeData[6] = t.day;
}
void updateTimeData(){
  //Time t(2013, 9, 22, 01, 38, 50, Time::kTuesday);
  Time t(timeData[0], timeData[1], timeData[2], timeData[3], timeData[4], timeData[5],  timeData[6]);
  rtc.time(t);
}
void addMinusOne(){
  
}

void formateDigit(){
    
}

/****
 * drawDisplay functions fordifferent interfaces
****/
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

void clockFace(){}
void setTime(){}
void setAlarm(){}
void countDown(){}
void stopWatch(){}
void scoreBoard(){}
void dice(){}


