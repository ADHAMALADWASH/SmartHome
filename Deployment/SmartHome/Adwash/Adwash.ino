#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Pins ----------
const int trigPin        = 12;
const int echoPin        = 13;
const int buzzerDistance = 6;
const int buzzerFlame    = 7;
const int flamePin       = 4;

const int relaySoilPin   = 8;   // Relay التربة (Active LOW)
const int relayLdrPin    = 9;   // Relay الإضاءة (Active LOW)

const int servo1Pin      = 3;
const int servo2Pin      = 5;

const int dhtPin         = 0;
const int soilPin        = A0;
const int ldrPin         = A1;

#define DHTTYPE DHT11

Servo servo1;
Servo servo2;
DHT dht(dhtPin, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Ultrasonic ----------
long duration;
float distance;
bool objectDetected = false;

// ---------- Soil Control ----------
int thresholdDry = 600;
int hysteresis   = 50;
unsigned long minOnMillis  = 2000;
unsigned long minOffMillis = 000;
unsigned long interval     = 3000;

bool relaySoilState = false;
unsigned long lastChange = 0;
unsigned long lastRead   = 0;

// ---------- LDR ----------
int ldrThreshold = 400;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buzzerDistance, OUTPUT);
  pinMode(buzzerFlame, OUTPUT);
  pinMode(flamePin, INPUT);

  pinMode(relaySoilPin, OUTPUT);
  pinMode(relayLdrPin, OUTPUT);

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo1.write(180);
  servo2.write(180);

  digitalWrite(relaySoilPin, HIGH);
  digitalWrite(relayLdrPin, HIGH);

  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
  unsigned long now = millis();

  int soilValue = analogRead(soilPin);
  int ldrValue  = analogRead(ldrPin);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // ---------- Soil Logic ----------
  if (now - lastRead >= interval) {
    lastRead = now;

    int onThreshold  = thresholdDry;
    int offThreshold = thresholdDry - hysteresis;

    if (!relaySoilState) {
      if (soilValue >= onThreshold && (now - lastChange) >= minOffMillis) {
        setSoilRelay(true);
        lastChange = now;
      }
    } else {
      if (soilValue <= offThreshold && (now - lastChange) >= minOnMillis) {
        setSoilRelay(false);
        lastChange = now;
      }
    }

    // ---------- Send to Python ----------
    unsigned long timeSeconds = millis() / 1000;

    String state = (soilValue >= thresholdDry) ? "dry" : "wet";

    Serial.print("SAND");              Serial.print("\t");
    Serial.print(state);               Serial.print("\t");
    Serial.print(soilValue);           Serial.print("\t");
    Serial.print(timeSeconds);         Serial.print("\t");
    Serial.print(relaySoilState ? 1 : 0); Serial.print("\t");
    Serial.println(ldrValue);
  } // <<< القوس الناقص كان هنا

  // ---------- LDR Relay ----------
  if (ldrValue < ldrThreshold)
    digitalWrite(relayLdrPin, LOW);

..3











































































































































































3.






































































































































































.2696+-13  else
    digitalWrite(relayLdrPin, HIGH);

  // ---------- Ultrasonic ----------
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 25000);
  distance = (duration == 0) ? 0 : duration * 0.034 / 2;

  if (distance > 0 && distance < 10 && !objectDetected) {
    servo1.write(90);
    servo2.write(90);
    digitalWrite(buzzerDistance, HIGH);
    objectDetected = true;
  }

  if (distance >= 10 && objectDetected) {
    servo1.write(180);
    servo2.write(180);
    digitalWrite(buzzerDistance, LOW);
    objectDetected = false;
  }

  // ---------- Flame ----------
  digitalWrite(buzzerFlame, digitalRead(flamePin) == LOW);

  // ---------- LCD ----------
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print(" H:");
  lcd.print(humidity);
  lcd.print("   ");

  lcd.setCursor(0,1);
  lcd.print("Soil:");
  lcd.print(soilValue);
  lcd.print("    ");

  delay(500);
}

void setSoilRelay(bool on) {
  digitalWrite(relaySoilPin, on ? LOW : HIGH);
  relaySoilState = on;
}
