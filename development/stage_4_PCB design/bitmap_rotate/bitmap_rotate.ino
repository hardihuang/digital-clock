#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

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

void setup() {
  Serial.begin(9600);
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 2);
  matrix.setRotation(3, 1);
  matrix.fillScreen(LOW); // show black
  matrix.write();
}
static unsigned char*  menuArray[1] = {countDown_bitmap};

void loop() {

  matrix.fillScreen(LOW);
  matrix.drawBitmap(17, 0, menuArray[0], 8, 8, 1);
  matrix.write();
  
  delay(1000);

  
}

void dice(){
  if(diceFlag){
    int diceValue = random(1,7);
    centerPrint(String(diceValue));
    delay(5000);
    diceFlag = 0;
  }else{
    centerPrint("SHAKE");
  }
}
