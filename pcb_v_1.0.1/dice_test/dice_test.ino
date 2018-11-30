#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int diceValue;

void setup() {
  Serial.begin(9600);
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  matrix.fillScreen(LOW); // show black
  matrix.write();
  randomSeed(analogRead(0));
}

void loop() {
  
  for(int i=1;i<10;i++){
    diceValue = random(1,7);
    displayDice(diceValue);
    delay(100);
  }
  displayDice(diceValue);
  delay(4000);
  
}

int displayDice(int diceValue){
  matrix.fillScreen(LOW);
  matrix.drawRect(12,1,1,5,1);
  matrix.drawRect(20,1,1,5,1);
  matrix.drawRect(13,0,7,1,1);
  matrix.drawRect(13,6,7,1,1); 
  switch(diceValue){
    case 1:
      matrix.drawPixel(16,3,1);
    break;  
    case 2:
      matrix.drawPixel(17,2,1); 
      matrix.drawPixel(15,4,1);
    break;
    case 3:
      matrix.drawPixel(18,2,1);
      matrix.drawPixel(16,3,1);
      matrix.drawPixel(14,4,1);
    break;
    case 4:
      matrix.drawPixel(14,2,1);
      matrix.drawPixel(14,4,1);
      matrix.drawPixel(18,2,1);
      matrix.drawPixel(18,4,1);
    break;
    case 5:
      matrix.drawPixel(14,2,1);
      matrix.drawPixel(14,4,1);
      matrix.drawPixel(18,2,1);
      matrix.drawPixel(18,4,1);
      matrix.drawPixel(16,3,1);
    break;
    case 6:
      matrix.drawPixel(14,2,1);
      matrix.drawPixel(14,4,1);
      matrix.drawPixel(18,2,1);
      matrix.drawPixel(18,4,1);
      matrix.drawPixel(16,2,1);
      matrix.drawPixel(16,4,1);
    break;
  }  
  matrix.write();
}
