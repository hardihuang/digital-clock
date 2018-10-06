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
{ B01111111,
  B01100011,
  B00110110,
  B00010100,
  B00010100,
  B00110110,
  B01100011,
  B01111111
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
  B01000111,
  B11000001,
  B01010001,
  B01000111,
  B01010001,
  B01000001,
  B11100111
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
int menuSelected =3;
String btn = "R";

void setup() {
  // put your setup code here, to run once:
  matrix.setIntensity(10);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  matrix.fillScreen(LOW); // show black
  Serial.begin(9600); 
}

void loop() {
  matrix.fillScreen(LOW); // show black
  matrix.drawBitmap(1,0,menuArray[menuSelected-1],8,8,1);
  matrix.drawBitmap(12,0,menuArray[menuSelected],8,8,1);
  matrix.drawBitmap(23,0,menuArray[menuSelected+1],8,8,1);
  matrix.write();
  delay(1000);

  if(btn == "L"){
    if(menuSelected>1){
      menuSelected--;
    }else{
      menuSelected = 6;  
    }
    menuAnimation(1);
  }else if(btn == "R"){
    
    if(menuSelected<6){
      menuSelected++;
    }else{
      menuSelected = 1;  
    }
    menuAnimation(-1);
  }
  
  delay(1000);
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

