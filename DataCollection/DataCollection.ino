const int sensorPin = A0;
const int relayPin  = 8;
int thresholdDry = 800;  
int pumpState = 0;
unsigned long lastRead = 0;
unsigned long interval = 60000; 
void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
}

void loop() {
  unsigned long now = millis();
  if (now - lastRead >= interval) {
    lastRead = now;
    int raw = analogRead(sensorPin);
    int moisturePercent = map(raw, 1023, 0, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);
    String state = (raw > thresholdDry) ? "dry" : "wet";
    if (state == "dry") {
      digitalWrite(relayPin, HIGH); 
      pumpState = 1;
    } else {
      digitalWrite(relayPin, LOW);  
      pumpState = 0;
    }
    Serial.print("sand,");
    Serial.print(state);
    Serial.print(",");
    Serial.print(moisturePercent);
    Serial.print(",");
    Serial.println(pumpState);
  }
}
