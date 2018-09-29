int photocellPin = 1;
int photocellReading;
int brightness;//1-15
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  photocellReading = analogRead(photocellPin);
  Serial.println(photocellReading);
  brightness = map(photocellReading,1023,0,1,15);
  Serial.println(brightness);
  delay(500);
  
}
