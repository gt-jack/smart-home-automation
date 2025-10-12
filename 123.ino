#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Pin Definitions ===
#define DHTPIN 3          //temp
#define DHTTYPE DHT11
#define LDR_DO_PIN 2         //ldr ard inp
#define RELAY_PIN 7          //ldr light
#define RELAY2_PIN 8          //fri light
#define FAN_PIN 5  //fan
#define MOTOR 9           
#define BUZZER_PIN 4        //buzzer
#define IR_PIN 6           //ir

// === Objects ===
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

String command = "";
bool silentMode = false;
unsigned long lastAlertTime = 0;
const int alertDuration = 3000;
String currentAlert = "";

void setup() {
  pinMode(LDR_DO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOTOR, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(MOTOR, LOW);

  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Booting System  ");
   delay(5000);
   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("JARVIS IS READY  ");
  delay(5000);
  lcd.clear();
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int ldrState = digitalRead(LDR_DO_PIN);
  int irState = digitalRead(IR_PIN);

  // === LDR Light Control ===
  if (ldrState == HIGH) {
    digitalWrite(RELAY_PIN, LOW); // Light ON when bright
    if (!silentMode) Serial.println("LDR: Bright → Light ON");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Light OFF when dark
    if (!silentMode) Serial.println("LDR: Dark → Light OFF");
  }

  // === IR Detection ===
  if (irState == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    
    
    if (!silentMode) Serial.println("IR: Motion Detected");
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // === High Temp Alert ===
  if (!isnan(temp) && temp > 40.0) {
    digitalWrite(BUZZER_PIN, HIGH);
    currentAlert = "HIGH TEMPERATURE ALERT ACTIVATED!";
    lastAlertTime = millis();
    if (!silentMode) Serial.println("TEMP: Overheat!");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // === LCD Display ===
  // === LCD Display ===
static unsigned long lastLcdUpdate = 0;
static float prevTemp = -1;
static float prevHum = -1;
static int prevLdr = -1;

if (millis() - lastAlertTime < alertDuration && currentAlert != "") {
  lcd.setCursor(0, 0);
  lcd.print(currentAlert.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(currentAlert.substring(16, 32));
} else if (millis() - lastLcdUpdate > 1000) {
  if (temp != prevTemp || hum != prevHum || ldrState != prevLdr) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp, 1);
    lcd.print("C H:");
    lcd.print(hum, 0);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Street Light:");
    lcd.print((ldrState == HIGH) ? "ON " : "OFF");

    prevTemp = temp;
    prevHum = hum;
    prevLdr = ldrState;
  }
  lastLcdUpdate = millis();
  currentAlert = "";
}


  // === Serial Commands ===
  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "relay2 on") {
      digitalWrite(RELAY2_PIN, LOW);
      if (!silentMode) Serial.println("FRIDAY: Manual Light ON");
    } else if (command == "relay2 off") {
      digitalWrite(RELAY2_PIN, HIGH);
      if (!silentMode) Serial.println("FRIDAY: Manual Light OFF");
    } else if (command == "fan on") {
      digitalWrite(FAN_PIN, LOW);
      digitalWrite(MOTOR , HIGH);
      if (!silentMode) Serial.println("FRIDAY: Fan ON");
      
    } else if (command == "fan off") {
      digitalWrite(FAN_PIN, HIGH);
      digitalWrite(MOTOR , LOW);
      if (!silentMode) Serial.println("FRIDAY: Fan OFF");
    } else if (command == "get temp") {
      silentMode = true; // Suppress debug prints
      float t = dht.readTemperature();
      if (!isnan(t)) {
        Serial.print("TEMP:");
        Serial.println(t, 1);
      } else {
        Serial.println("TEMP:Error");
      }
      silentMode = false;
    }
  }

  delay(100);  // Prevent flooding
}
