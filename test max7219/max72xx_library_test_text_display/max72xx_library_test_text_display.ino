#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int timer;

void setup() {

  matrix.setIntensity(1); // Use a value between 0 and 15 for brightness

  // Adjust to your own needs
  //  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  //  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  //  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
  //  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>
  //  ...
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
}

void loop() {

  matrix.fillScreen(LOW);
  matrix.drawChar(1, 0, '2', HIGH, LOW, 1);
  matrix.drawChar(8, 0, '2', HIGH, LOW, 1);
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
  
  //matrix.drawChar(13, 0, ':', HIGH, LOW, 1);              
  matrix.drawChar(19, 0, '3', HIGH, LOW, 1);
  matrix.drawChar(26, 0, '5', HIGH, LOW, 1);
  matrix.write();
  delay(500);
}

