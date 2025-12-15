#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// عدّل العنوان حسب I2C Scanner (0x27 أو 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();        // تهيئة الشاشة
  lcd.backlight();   // تشغيل الإضاءة الخلفية
}

void loop() {
  // السطر الأول
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Green House");

  // السطر الثاني مع OK
  lcd.setCursor(0, 1);
  lcd.print("Dry time 11h:51");
  delay(5000);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("NO ERROR");
  // انتظر 5 ثواني قبل التحويل إلى LDR ERR
  delay(5000);
  lcd.clear();
  // مسح OK وكتابة LDR ERR
  lcd.setCursor(5, 1);
  lcd.print("Error LDR");   // مسح النص القديم
 

  // انتظر 3 ثواني قبل إعادة التحديث
  delay(3000);
}
