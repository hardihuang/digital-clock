#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <stdio.h>
int pinCS = 10;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int countDownData[6]={0,59,0,1,59,0};//hour,minute,second
double intervalSpeed;
unsigned long timer = millis();
int realTime;
int countDownPast;
unsigned int pn;
int row;
int left;

void setup() {
  Serial.begin(9600);
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  matrix.fillScreen(LOW); // show black
  matrix.write();
  intervalSpeed = (countDownData[1]*60000+countDownData[2]*1000)/256.0;
}

void loop() {
  Serial.print("interval speed: ");
  Serial.print(intervalSpeed); 
  Serial.print("|| minute: ");
  Serial.print(countDownData[4]);
  Serial.print("|| second: ");
  Serial.print(countDownData[5]);
  
  pn = countDownData[4]*(60000/intervalSpeed) + countDownData[5]*(1000/intervalSpeed);//current pixel number
  Serial.print("|| pixel number: ");
  Serial.println(pn); 
  row = pn/8;
  left = pn % 8;
/*
  Serial.print("interval speed: ");
  Serial.print(intervalSpeed); 
  Serial.print("|| pixel number: ");
  Serial.print(pn); 
  Serial.print("|| Row: ");
  Serial.print(row);
  Serial.print("|| Left: ");
  Serial.print(left);
  Serial.print("|| minute: ");
  Serial.print(countDownData[4]);
  Serial.print("|| second: ");
  Serial.println(countDownData[5]);
*/
  matrix.fillScreen(LOW);
  if(row>0){
    matrix.fillRect(0,0,row,8,1);  
  }
  if(left>0){
    matrix.fillRect(row,0,1,left,1);  
  }
  matrix.write();
  
  if(millis()-timer>=100){
    if(countDownData[5]>0){
      countDownData[5]--;
    }else if(countDownData[4]>0){
      countDownData[4]--;
      countDownData[5]=59;
    }else{
      countDownData[4]=countDownData[1];
      countDownData[5]=countDownData[2]; 
    }
    timer = millis();
  }
  delay(100);
}
