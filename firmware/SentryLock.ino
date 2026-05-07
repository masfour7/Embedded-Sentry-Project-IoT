/*
 * =====================================================================
 *  Embedded Sentry — Gesture-Based Hardware Lock
 *  Author : Mohammad Asfour
 *  Course : NYU Embedded Systems (Spring 2020) · Prof. Campisi
 *
 *  Hardware
 *    - ATmega328p (Arduino Uno/Nano)
 *    - MPU6050    (I²C, addr 0x68)  — SDA=A4, SCL=A5
 *    - LED        on D13            — represents the unlocked resource
 *
 *  How it works
 *    A finite-state machine reads accelerometer data, classifies each
 *    "flick" of the wrist into one of six discrete directions, then:
 *      1) RECORD  — captures SEQ_LEN moves as the secret key
 *      2) ARMED   — waits for the user to repeat the same SEQ_LEN moves
 *      3) UNLOCK  — drives the LED HIGH for UNLOCK_MS before re-arming
 * =====================================================================
 */

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// ----- Config ---------------------------------------------------------
constexpr uint8_t  LED_PIN          = 13;
constexpr uint8_t  SEQ_LEN          = 3;       // number of moves in the secret
constexpr uint16_t DEBOUNCE_MS      = 500;     // gap between gesture reads
constexpr uint16_t UNLOCK_MS        = 3000;    // how long the LED stays on
constexpr uint16_t LOOP_DELAY_MS    = 20;

// Accelerometer thresholds (m/s²) — tuned empirically on MPU6050 @ ±8G
// Note on axes: with the breakout sitting flat, gravity (~9.81) reads on Z.
//   X axis → left/right tilt
//   Z axis → up/down tilt (deviation from gravity rest of ~9.81)
//   Y axis → forward/backward tilt
constexpr float TH_LR        = 4.0f;   // |x| over this  -> left/right
constexpr float TH_UP        = 13.0f;  //  z over this   -> up
constexpr float TH_DOWN      = 7.0f;   //  z under this  -> down
constexpr float TH_FB        = 3.0f;   // |y| over this  -> forward/back
constexpr float TH_LR_GUARD  = 2.0f;   // |x| under this for fwd/back to count

// ----- Direction codes -----------------------------------------------
enum Direction : uint8_t {
  DIR_NONE     = 0,
  DIR_RIGHT    = 1,
  DIR_LEFT     = 2,
  DIR_UP       = 3,
  DIR_DOWN     = 4,
  DIR_FORWARD  = 5,
  DIR_BACKWARD = 6,
};

const char* dirName(Direction d) {
  switch (d) {
    case DIR_RIGHT:    return "right";
    case DIR_LEFT:     return "left";
    case DIR_UP:       return "up";
    case DIR_DOWN:     return "down";
    case DIR_FORWARD:  return "forward";
    case DIR_BACKWARD: return "backward";
    default:           return "none";
  }
}

// ----- FSM phases -----------------------------------------------------
enum Phase : uint8_t { PH_RECORD, PH_ARMED, PH_UNLOCKED };

// ----- State ----------------------------------------------------------
Adafruit_MPU6050 mpu;
Phase     phase            = PH_RECORD;
Direction secret[SEQ_LEN]  = {};
uint8_t   recordIdx        = 0;
uint8_t   attemptIdx       = 0;

// ---------------------------------------------------------------------
// Classify a single accelerometer sample into a discrete direction.
// Returns DIR_NONE if no axis crosses its threshold this tick.
// ---------------------------------------------------------------------
Direction classify(float ax, float ay, float az) {
  if (ax >  TH_LR) return DIR_LEFT;
  if (ax < -TH_LR) return DIR_RIGHT;
  if (az >  TH_UP) return DIR_UP;
  if (az <  TH_DOWN) return DIR_DOWN;
  if (ay >  TH_FB && fabsf(ax) < TH_LR_GUARD) return DIR_BACKWARD;
  if (ay < -TH_FB && fabsf(ax) < TH_LR_GUARD) return DIR_FORWARD;
  return DIR_NONE;
}

// ---------------------------------------------------------------------
// Read and debounce a single gesture from the accelerometer.
// ---------------------------------------------------------------------
Direction readGesture() {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  // Pass raw X/Y/Z; classify() understands the axis semantics (X=LR, Z=UD, Y=FB).
  Direction d = classify(a.acceleration.x, a.acceleration.y, a.acceleration.z);
  if (d != DIR_NONE) {
    Serial.println(dirName(d));
    delay(DEBOUNCE_MS);
  }
  return d;
}

// ---------------------------------------------------------------------
// Drive the unlock signal on the LED pin.
// ---------------------------------------------------------------------
void openLock() {
  Serial.println(F("Opened! Closing in 3s..."));
  digitalWrite(LED_PIN, HIGH);
  delay(UNLOCK_MS);
  digitalWrite(LED_PIN, LOW);
  Serial.println(F("Closed again. Enter sequence to open..."));
}

// ---------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  while (!Serial) delay(10);
  Serial.println(F("== Embedded Sentry =="));

  if (!mpu.begin()) {
    Serial.println(F("MPU6050 not found — check wiring."));
    while (true) delay(100);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.print(F("Record ")); Serial.print(SEQ_LEN);
  Serial.println(F(" gestures to set the secret."));
}

// ---------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------
void loop() {
  Direction d = readGesture();

  switch (phase) {

    case PH_RECORD:
      if (d == DIR_NONE) break;
      secret[recordIdx++] = d;
      Serial.print(F("recorded ")); Serial.print(recordIdx);
      Serial.print(F("/")); Serial.println(SEQ_LEN);
      if (recordIdx >= SEQ_LEN) {
        phase = PH_ARMED;
        attemptIdx = 0;
        Serial.println(F("Armed. Repeat sequence to unlock."));
      }
      break;

    case PH_ARMED:
      if (d == DIR_NONE) break;
      if (d == secret[attemptIdx]) {
        Serial.print(F("match ")); Serial.print(attemptIdx + 1);
        Serial.print(F("/")); Serial.println(SEQ_LEN);
        if (++attemptIdx >= SEQ_LEN) {
          phase = PH_UNLOCKED;
        }
      } else {
        Serial.println(F("Wrong key — start over."));
        attemptIdx = 0;
      }
      break;

    case PH_UNLOCKED:
      openLock();
      attemptIdx = 0;
      phase = PH_ARMED;
      break;
  }

  delay(LOOP_DELAY_MS);
}
