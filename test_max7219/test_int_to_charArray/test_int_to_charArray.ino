int timeData[7] = {2000,9,27,8,57,32,25};
String tempStr;
char tempHr[2];

void setup() {
  // put your setup code here, to run once:
  tempStr = String(timeData[3]);
  tempStr.toCharArray(tempHr,3);
  Serial.begin(9600);
  
  Serial.println(timeData[3]);
  Serial.println(tempHr[0]);
  Serial.println(tempHr[1]);
}

void loop() {
  // put your main code here, to run repeatedly:

}
