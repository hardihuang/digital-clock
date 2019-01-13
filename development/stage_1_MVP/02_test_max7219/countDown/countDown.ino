#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
#include <DS1302.h>
int buzzPin = 9;
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
char timeArr[8][32] ={
 //1                         14 
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//1
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//2
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//3
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//4
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//5
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//6
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//7
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //8
};

void setup(){
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);
  matrix.setIntensity(10);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1); 
  Serial.begin(9600); 
}

void loop(){
  drawDisplay();
  delay(1000);
  matrix.fillScreen(LOW);
  for(int m = 0;m<14;m++){
    for(int n=0;n<8;n++){
      timeArr[n][13-m] = 0;
      timeArr[n][17-m] = 1;  
      drawDisplay();
      
      delay(5);
      timeArr[n][17-m] = 0;
      timeArr[n][21-m] = 1;  
      drawDisplay();
      delay(2);
      tone(buzzPin, 500, 5);
      timeArr[n][21-m] = 0;
      timeArr[n][26-m] = 1;  
      drawDisplay();
      delay(2);
      timeArr[n][26-m] = 0;
      timeArr[n][31-m] = 1;  
      drawDisplay();
      noTone(buzzPin);
      delay(1000);
      
    }  
  }
  resetTimeArr();
  
}

void drawDisplay(){
  for (int i = 0;i<32;i++){
    for(int j=0;j<8;j++){
      matrix.drawPixel(i,j,timeArr[j][i]);  
    }
  } 
  matrix.write(); 

}

void resetTimeArr(){
  for(int p = 0;p<32;p++){
    for(int q=0;q<8;q++){
      if(p<14){
        timeArr[q][p] = 1;  
      }else{
        timeArr[q][p] = 0;  
      }
    }  
  }
}









