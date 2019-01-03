#include <EEPROM.h>
int data[5]={0,0,0,0,0};
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  writeData();
  fetchData();
  for(int i=0;i<5;i++){
    Serial.print(" || ");  
    Serial.print(data[i]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void writeData() {
  for (int i = 0; i < 5; i++) {
    EEPROM.write(i, data[i]);
  }
}

void fetchData() {
  for (int i = 0; i < 5; i++) {
    data[i] = EEPROM.read(i);
  }
}
