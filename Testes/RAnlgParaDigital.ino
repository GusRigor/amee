unsigned int adc;

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  Serial.printf("Iniciando...\n");
  
  
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(100);
}

void setup(){
  adc=0;
  pinMode(2,INPUT);
  
  while(digitalRead(2) == LOW)
    adc++;
  Serial.printf("%i\n", adc);
  
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(100);
  
}
