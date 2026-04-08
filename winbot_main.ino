// ============================================================
//  WINBOT Autonomous Window Cleaning System
//  Arduino Nano + 3x L298N + 4x TT Gear Motors + Winch Motor
//
//  Authors:
//    - Human Project Lead (hardware, testing, calibration)
//    - Claude AI (Anthropic) — code architecture & documentation
//      https://claude.ai
//
//  Repository: github.com/YOUR_USERNAME/winbot-autonomous-cleaner
//  License: MIT
//  Version: 4.0 — April 2026
//
// ============================================================
//  L298N #1 (Cart motors — left side):
//  ENA1 → D5  IN1 → D6   IN2 → D7
//  ENB1 → D3  IN3 → D2   IN4 → D4
//
//  L298N #2 (Cart motors — right side):
//  ENA2 → D9  IN5 → D8   IN6 → D10
//  ENB2 → D11 IN7 → D12  IN8 → D13
//
//  L298N #3 (Winch/pulley motor):
//  ENA3 → A0  IN9 → A1   IN10 → A2
//
//  POWER (all in parallel):
//  9V battery (+) → L298N #1,#2,#3 +12V terminals + Nano VIN
//  9V battery (-) → L298N #1,#2,#3 GND            + Nano GND
//
//  HOW TO CALIBRATE:
//  1. Set CALIBRATION_CM_PER_SEC by running cart at PWM 255
//     and measuring how many cm it travels per second
//  2. Set COLUMN_SPAN_CM to the width between vertical columns
//  3. Set ROOF_LENGTH_CM to total horizontal travel distance
//  4. Speed and travel time are calculated automatically
//
//  TO RE-RUN: press RESET button on Nano
// ============================================================

// ── L298N #1 pins (cart left) ────────────────────────────────
const int ENA1 = 5;
const int IN1  = 6;
const int IN2  = 7;
const int ENB1 = 3;
const int IN3  = 2;
const int IN4  = 4;

// ── L298N #2 pins (cart right) ───────────────────────────────
const int ENA2 = 9;
const int IN5  = 8;
const int IN6  = 10;
const int ENB2 = 11;
const int IN7  = 12;
const int IN8  = 13;

// ── L298N #3 pins (winch) ────────────────────────────────────
const int ENA3 = A0;
const int IN9  = A1;
const int IN10 = A2;

// ══ USER DEFINED PARAMETERS — edit these ════════════════════
const float COLUMN_SPAN_CM          = 50.0;  // width between columns (cm)
const float ROOF_LENGTH_CM          = 200.0; // total horizontal travel (cm)
const float CALIBRATION_CM_PER_SEC  = 16.6;  // cart speed at PWM 255 (measure this!)
const int   WINCH_SPEED             = 180;   // winch motor PWM speed (0–255)
// ════════════════════════════════════════════════════════════

// ── Calculated at startup ────────────────────────────────────
int cartPWM              = 0;
unsigned long travelTimeMS = 0;

bool stopped         = false;
unsigned long startTime = 0;

void setup() {
  // L298N #1
  pinMode(ENA1, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB1, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // L298N #2
  pinMode(ENA2, OUTPUT); pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(ENB2, OUTPUT); pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);

  // L298N #3 winch
  pinMode(ENA3, OUTPUT); pinMode(IN9, OUTPUT); pinMode(IN10, OUTPUT);

  Serial.begin(9600);
  Serial.println("=== WINBOT Autonomous Cleaning System — Ready ===");

  // ── Calculate speed PWM from column span ──────────────────
  float targetSpeedCmPerSec = COLUMN_SPAN_CM / 1.0;
  cartPWM = (int)((targetSpeedCmPerSec / CALIBRATION_CM_PER_SEC) * 255.0);
  cartPWM = constrain(cartPWM, 0, 255);

  // ── Calculate travel time from roof length ────────────────
  float cartSpeedCmPerSec = (cartPWM / 255.0) * CALIBRATION_CM_PER_SEC;
  travelTimeMS = (unsigned long)((ROOF_LENGTH_CM / cartSpeedCmPerSec) * 1000.0);

  // ── Print calculated values ───────────────────────────────
  Serial.println("-------------------------------------------------");
  Serial.print("Column span:       "); Serial.print(COLUMN_SPAN_CM);    Serial.println(" cm");
  Serial.print("Roof length:       "); Serial.print(ROOF_LENGTH_CM);    Serial.println(" cm");
  Serial.print("Calculated PWM:    "); Serial.println(cartPWM);
  Serial.print("Cart speed:        "); Serial.print(cartSpeedCmPerSec); Serial.println(" cm/s");
  Serial.print("Travel time:       "); Serial.print(travelTimeMS);      Serial.println(" ms");
  Serial.print("Winch PWM:         "); Serial.println(WINCH_SPEED);
  Serial.println("-------------------------------------------------");

  stopAll();
  delay(500);

  moveForward(cartPWM);
  winchPayout(WINCH_SPEED);
  startTime = millis();
  Serial.println("Moving forward + winch paying out...");
}

void loop() {
  if (stopped) return;

  unsigned long elapsed = millis() - startTime;

  if (elapsed >= travelTimeMS) {
    stopAll();
    stopped = true;
    Serial.println("[STOP] Roof length reached — cycle complete.");
    Serial.println("Press RESET button to run again.");
    return;
  }

  // Print progress every 500ms
  if (elapsed % 500 < 50) {
    int percent = (int)((elapsed * 100UL) / travelTimeMS);
    float distanceCovered = (elapsed / 1000.0) *
                            ((cartPWM / 255.0) * CALIBRATION_CM_PER_SEC);
    Serial.print("Progress: ");
    Serial.print(percent);
    Serial.print("% — ~");
    Serial.print(distanceCovered, 1);
    Serial.print(" cm / ");
    Serial.print(ROOF_LENGTH_CM);
    Serial.println(" cm");
  }
}

// ─────────────────────────────────────────────────────────────

void moveForward(int speed) {
  // L298N #1
  analogWrite(ENA1, speed); analogWrite(ENB1, speed);
  digitalWrite(IN1, HIGH);  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);  digitalWrite(IN4, LOW);

  // L298N #2
  analogWrite(ENA2, speed); analogWrite(ENB2, speed);
  digitalWrite(IN5, HIGH);  digitalWrite(IN6, LOW);
  digitalWrite(IN7, HIGH);  digitalWrite(IN8, LOW);
}

void winchPayout(int speed) {
  analogWrite(ENA3, speed);
  digitalWrite(IN9,  HIGH);
  digitalWrite(IN10, LOW);
}

void winchRetract(int speed) {
  analogWrite(ENA3, speed);
  digitalWrite(IN9,  LOW);
  digitalWrite(IN10, HIGH);
}

void stopAll() {
  // Cart motors
  analogWrite(ENA1, 0); analogWrite(ENB1, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);

  analogWrite(ENA2, 0); analogWrite(ENB2, 0);
  digitalWrite(IN5, LOW); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW); digitalWrite(IN8, LOW);

  // Winch
  analogWrite(ENA3, 0);
  digitalWrite(IN9,  LOW);
  digitalWrite(IN10, LOW);
}
