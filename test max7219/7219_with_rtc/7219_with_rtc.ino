#include <LiquidCrystal.h>
#include <stdio.h>
#include <DS1302.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

namespace {
const int kCePin   = 8;  //RST
const int kIoPin   = 6;  //DAT
const int kSclkPin = 7;  //CLK
DS1302 rtc(kCePin, kIoPin, kSclkPin);
String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sun";
    case Time::kMonday: return "Mon";
    case Time::kTuesday: return "Tue";
    case Time::kWednesday: return "Wed";
    case Time::kThursday: return "Thu";
    case Time::kFriday: return "Fri";
    case Time::kSaturday: return "Sat";
  }
  return "(unknown day)";
}
}

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  //setTime(); 
  lcd.begin(16, 2);
  Serial.begin(9600);  
}

void loop() {
  Time t = rtc.time();
  
  const String day = dayAsString(t.day);
  String year;
  String month;
  String date;
  String hour;
  String minutes;
  String second;

  year = String(t.yr);
  month = String(t.mon);
  date = String(t.date);
  hour = String(t.hr);
  minutes = String(t.min);
  second = String(t.sec);


  if(t.hr<10){
    hour = "0"+hour;
  }
  if(t.min<10){
    minutes = "0"+minutes;
  }
  if(t.sec<10){
    second = "0"+second;
  }

  //Serial.println(fullTime);
  //Serial.println(fullDate);

  
  //Serial.println(hour.substring(0,1));
  //Serial.println(hour.substring(1,2));
  char tempHourTen = hour.charAt(0);
  char tempHourOne = hour.charAt(1);
  char tempMinuteTen = minutes.charAt(0);
  char tempMinuteOne = minutes.charAt(1);

  Serial.println(tempHourTen);
  Serial.println(tempHourOne);
  Serial.println(tempMinuteTen);
  Serial.println(tempMinuteOne);
  //delay(1000);

  matrix.fillScreen(LOW);
  matrix.drawChar(1, 0, tempHourTen, HIGH, LOW, 1);
  matrix.drawChar(8, 0, tempHourOne, HIGH, LOW, 1);
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
  matrix.drawChar(19, 0, tempMinuteTen, HIGH, LOW, 1);
  matrix.drawChar(26, 0, tempMinuteOne, HIGH, LOW, 1);
  matrix.write();
  delay(500);
}

void setTime()
{
  rtc.writeProtect(false);
  rtc.halt(false);
  Time t(2015, 2, 26, 13, 35, 50, Time::kThursday); //Change this line to set time  ex 2015 26/2 9:09:50
  rtc.time(t);
}
