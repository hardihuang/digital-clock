int timeData[7] = {2000,9,27,18,57,32,25};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  drawDisplay();
  delay(1000);
}

void drawDisplay(){
  String tempStr;
  char tempHr[2];
  char tempMin[2];
  char tempSec[2];

  tempStr="";
  tempStr = String(timeData[4]);
  if(timeData[4]<10){
    tempStr="0"+tempStr;
  }
  tempStr.toCharArray(tempMin,3);
  tempStr="";
  
  tempStr = String(timeData[3]);  
  if(timeData[3]<10){
    tempStr="0"+tempStr;
  }
  tempStr.toCharArray(tempHr,3);
  tempStr="";
  
  tempStr = String(timeData[5]); 
  if(timeData[5]<10){
    tempStr="0"+tempStr;
  }
  tempStr.toCharArray(tempSec,3);
  tempStr="";
  
  Serial.println(timeData[0]);
  Serial.println(timeData[1]);
  Serial.println(timeData[2]);
  Serial.println(timeData[3]);
  Serial.println(tempHr[0]);
  Serial.println(tempHr[1]);
  Serial.println(timeData[4]);
  Serial.println(tempMin[0]);
  Serial.println(tempMin[1]);
  Serial.println(timeData[5]);
  Serial.println(tempSec[0]);
  Serial.println(tempSec[1]);
  Serial.println(timeData[6]);

}
