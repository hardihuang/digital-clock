#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
#include <DS1302.h>
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

static const unsigned char zero_bitmap[] =
{ B111,
  B101,
  B101,
  B101,
  B101,
  B101,
  B111
};

static const unsigned char one_bitmap[] =
{ 111,
  101,
  101,
  101,
  101,
  101,
  111
};

static unsigned char*  newFont[2] ={zero_bitmap,one_bitmap};

void setup(){
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1); 
  Serial.begin(9600); 
}

void loop(){
  matrix.fillScreen(LOW);
  matrix.drawBitmap(0,0,newFont[0],3,7,1);
  matrix.write(); 

  //Serial.println(invader1a[0][0]);
  delay(1000);
}










