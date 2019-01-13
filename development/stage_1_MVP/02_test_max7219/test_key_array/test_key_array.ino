String key="0";
int debounce[3];
char hexaKeys[]={'L','R','S'};
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  
  if(key!="0"){
    Serial.println(key);
    key = "0";
  }
  int keyArray[] = {4,3,2};//left, right, set
  for(int i = 0; i < 3; i++){ 
    if(digitalRead(keyArray[i]) == 1){
      if(debounce[i] == 0){
        key = hexaKeys[i];
        debounce[i] = 10;  
      }else{
        debounce[i] -= 1;  
      }
    }else{
      debounce[i] = 0;
    }
  }
delay(100);
  
}
