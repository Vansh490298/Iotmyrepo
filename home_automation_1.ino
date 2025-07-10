#include <LiquidCrystal.h>
#include <Servo.h>

// LCD pin configuration: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Sensor and device pins
const int TRIG_PIN = 7;
const int ECHO_PIN = 6;
const int RELAY_PIN = 13;
const int PIR_PIN = 8;
const int LDR_PIN = A0;
const int GAS_SENSOR_PIN = A1;
const int SERVO_PIN = 9;
const int BUZZER_PIN = 10;

// Thresholds
const int LDR_THRESHOLD = 600;
const int DISTANCE_TRIGGER = 40; // cm
const int GAS_ALARM_THRESHOLD = 300;

int cm = 0;
Servo myServo;

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH, 30000); // Timeout to prevent hanging
}

void setup() {
  lcd.begin(16, 2);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0); // Initial servo position

  Serial.begin(9600);
}

void loop() {
  // --- Distance Sensor ---
  cm = 0.01723 * readUltrasonicDistance(TRIG_PIN, ECHO_PIN);
  lcd.setCursor(0, 0);
  lcd.print("D:");
  lcd.print(cm);
  lcd.print("cm ");

  // --- Servo Control + Door Status ---
  lcd.setCursor(8, 1); // Right side, second row
  if (cm > 0 && cm <= DISTANCE_TRIGGER) {
    myServo.write(90); // Door Open
    lcd.print("DoorOpen ");
  } else {
    myServo.write(0);  // DoorClose
    lcd.print("DoorClose");
  }

  // --- Motion + LDR + Light Control ---
  int motion = digitalRead(PIR_PIN);
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("LDR: ");
  Serial.println(ldrValue);

  lcd.setCursor(0, 1); // Left side of second row
  if (ldrValue < LDR_THRESHOLD) {
    if (motion == HIGH) {
      digitalWrite(RELAY_PIN, HIGH);
      lcd.print("L: On ");
      delay(5000); // Keep light on for 5 seconds
    } else {
      digitalWrite(RELAY_PIN, LOW);
      lcd.print("L: Off");
      delay(300);
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    lcd.print("L: Off");
    delay(500);
  }

  // --- Gas Sensor + Buzzer ---
  int gasValue = analogRead(GAS_SENSOR_PIN);
  lcd.setCursor(8, 0); // Right side, top row

  if (gasValue <= 85) {
    lcd.print("G:Low ");
  } else if (gasValue <= 120) {
    lcd.print("G:Med ");
  } else if (gasValue <= 200) {
    lcd.print("G:High");
  } else if (gasValue <= GAS_ALARM_THRESHOLD) {
    lcd.print("G:Ext ");
  } else {
    lcd.print("G:Alrm");
  }

  // Buzzer ON if gas is too high
  if (gasValue > GAS_ALARM_THRESHOLD) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(250); // LCD refresh rate
}
