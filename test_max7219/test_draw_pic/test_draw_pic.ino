#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
#include <DS1302.h>
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
char menuGraph[8][32] =
{
  {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},//1
  {0,0,0,0,0,1,0,1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},//2
  {0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0},//3
  {0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0},//4
  {1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},//5
  {0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0},//6
  {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},//7
  {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0}//8
};

void setup(){
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1); 
  Serial.begin(9600); 
}

void loop(){

  for (int i = 0;i<32;i++){
    for(int j=0;j<8;j++){
      matrix.drawPixel(i,j,menuGraph[j][i]);  
    }
  } 
  matrix.write(); 

  //Serial.println(invader1a[0][0]);
  delay(1000);
}










