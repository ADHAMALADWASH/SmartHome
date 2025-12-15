const int moisturePin = A0;  // دخل حساس الرطوبة التناظري
const int pumpPin = 7;       // خرج الريليه للمضخة
const int moistureThreshold = 600;

unsigned long previousMillis = 0;
const long interval = 1000; // 10 ثواني

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW); // المضخة مغلقة في البداية
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // قراءة الرطوبة
    int moistureValue = analogRead(moisturePin);

    // حالة الرطوبة
    String state = (moistureValue > moistureThreshold) ? "wet" : "dry";

    // soil_type ثابت
    String soil_type = "sand";

    // حالة المضخة
    int pumpState = 0;

    // تشغيل المضخة لو الرطوبة أقل من threshold
    if (moistureValue < moistureThreshold) {
      digitalWrite(pumpPin, HIGH);
      pumpState = 1;
    } else {
      digitalWrite(pumpPin, LOW);
      pumpState = 0;
    }

    // تحويل millis() لصيغة HH:MM:SS
    unsigned long totalSeconds = currentMillis / 1000;
    unsigned long hours = (totalSeconds / 3600) % 24;
    unsigned long minutes = (totalSeconds / 60) % 60;
    unsigned long seconds = totalSeconds % 60;

    char timeStr[9];
    sprintf(timeStr, "%02lu:%02lu:%02lu", hours, minutes, seconds);

    // طباعة الداتا على Serial
    Serial.print(soil_type); Serial.print("\t");
    Serial.print(state); Serial.print("\t");
    Serial.print(moistureValue); Serial.print("\t");
    Serial.print(timeStr); Serial.print("\t");
    Serial.println(pumpState);
  }
}
