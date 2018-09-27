#include <LiquidCrystal.h>
#include <stdio.h>
#include <DS1302.h>


namespace {

const int kCePin   = 8;  //RST
const int kIoPin   = 6;  //DAT
const int kSclkPin = 7;  //CLK

// Create a DS1302 object.
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
  //setTime(); 
  lcd.begin(16, 2);
  Serial.begin(9600);  
}

void loop() {

 printTime();
 delay(1000);
}


void printTime() {
  
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
  String fullDate =" "+year+" "+month+"/"+date+"  "+day;
  
  if(t.hr<10){
    hour = "0"+hour;
  }
  if(t.min<10){
    minutes = "0"+minutes;
  }
  if(t.sec<10){
    second = "0"+second;
  }
  String fullTime = "    "+hour+":"+minutes+":"+second;
  
  lcd.setCursor(0,0);
  lcd.print(fullDate);
  lcd.setCursor(0,1);
  lcd.print(fullTime);
}

void setTime()
{
  rtc.writeProtect(false);
  rtc.halt(false);
  Time t(2015, 2, 26, 13, 35, 50, Time::kThursday); //Change this line to set time  ex 2015 26/2 9:09:50
  rtc.time(t);
}
