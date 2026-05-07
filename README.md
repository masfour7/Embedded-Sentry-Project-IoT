<div align="center">

# 🔐 Embedded Sentry

### A gesture-based hardware lock built with an Arduino + MPU6050 accelerometer

[![Made with C](https://img.shields.io/badge/Made%20with-C%20%2F%20Arduino-00979D?logo=arduino&logoColor=white)](#)
[![MCU](https://img.shields.io/badge/MCU-ATmega328p-blue)](#)
[![Sensor](https://img.shields.io/badge/Sensor-MPU6050-7c5cff)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Live Demo](https://img.shields.io/badge/▶-Try%20the%20live%20demo-22d3ee)](#-try-it-in-your-browser)

> **Wave your hand. Unlock the world.**
> Record a secret 3-move gesture sequence; the device only unlocks when that exact motion is repeated.

</div>

---

## 🎯 What it does

Embedded Sentry is a wearable security primitive: a microcontroller reads a 3-axis
accelerometer, classifies each flick of the wrist into one of six discrete directions
(`← → ↑ ↓ forward backward`), and uses a finite-state machine to:

1. **Record** a secret sequence of `N` gestures.
2. **Arm** itself and wait for someone to repeat it.
3. **Drive an unlock signal** (an LED on Pin 13 — easily swapped for a relay) for 3 seconds when the sequence matches.

It's tiny, hardware-only, and needs no internet, no companion app, and no key fob.

---

## ✨ Try it in your browser

There's a **fully interactive web simulator** in [`web-demo/`](web-demo/) that runs the same finite-state machine as the firmware — no hardware needed.

```bash
# from the repo root
cd web-demo && python3 -m http.server 5000
# then open http://localhost:5000
```

The simulator implements the **same finite-state machine** as the firmware (record → arm → unlock), with a live accelerometer readout, animated tilting device, and real-time serial log. Click the directional buttons or use the arrow keys + `W` / `S` to flick the device — the firmware classifies gestures from raw accelerometer samples; the demo skips that step and feeds discrete directions straight into the FSM.

---

## 🧠 How it works

### Finite-state machine

```
       ┌──────────────┐  N gestures   ┌────────────┐  match all  ┌────────────┐
 ────► │   RECORD     │ ────────────► │   ARMED    │ ──────────► │  UNLOCKED  │
       │  (capture)   │               │  (verify)  │             │ (LED HIGH) │
       └──────────────┘               └─────┬──────┘             └──────┬─────┘
                                            │ wrong move                │ 3 s
                                            ▼                           │
                                       (reset attempt) ◄────────────────┘
```

### Direction classification

The MPU6050 reports linear acceleration in m/s² on three axes. At rest the device reads `(0, 0, 9.81)` — pure gravity. A flick of the wrist briefly pushes one axis past a threshold:

| Gesture   | Condition                |
|-----------|--------------------------|
| Right     | `x < -4`                 |
| Left      | `x >  4`                 |
| Up        | `z >  13`                |
| Down      | `z <   7`                |
| Forward   | `y < -3` and `\|x\| < 2` |
| Backward  | `y >  3` and `\|x\| < 2` |

A 500 ms debounce ensures one physical flick maps to exactly one symbol.

---

## 📂 Repository layout

```
.
├── firmware/
│   └── SentryLock.ino                ← clean, table-driven implementation (recommended)
├── web-demo/
│   ├── index.html                    ← interactive in-browser simulator
│   ├── styles.css
│   └── app.js                        ← same FSM, in JavaScript
├── docs/
│   └── WIRING.md                     ← hardware bill of materials + pinout
├── Project_with_keyboard.ino         ← original NYU submission (kept for history)
├── Project_with_no_keyboard.ino      ← original NYU submission (kept for history)
└── LICENSE
```

---

## 🔌 Hardware

| Part            | Notes                              |
|-----------------|------------------------------------|
| ATmega328p      | Arduino Uno / Nano                 |
| MPU6050         | I²C, address `0x68` (SDA=A4, SCL=A5) |
| LED + 220 Ω     | On D13 — swap for a relay to drive a real lock |

Full wiring diagram and pin map: [`docs/WIRING.md`](docs/WIRING.md).

---

## 🚀 Flashing the firmware

1. Install the Arduino IDE (or `arduino-cli`).
2. Add the **Adafruit MPU6050** library via the Library Manager (it pulls `Adafruit_Sensor` automatically).
3. Open `firmware/SentryLock.ino`, select your board + port, and click **Upload**.
4. Open the Serial Monitor at **115200 baud** to see the FSM transitions.

---

## 🛠 Tuning

All thresholds are `constexpr` at the top of `SentryLock.ino`:

```cpp
constexpr uint8_t  SEQ_LEN     = 3;     // length of the secret
constexpr uint16_t DEBOUNCE_MS = 500;   // delay between gestures
constexpr uint16_t UNLOCK_MS   = 3000;  // how long the LED stays on
constexpr float    TH_LR       = 4.0f;  // accel threshold for left/right
// ...
```

Want a 5-move sequence? Change `SEQ_LEN`. Want a more sensitive flick? Lower the thresholds.

---

## 📜 License

[MIT](LICENSE) — built originally for NYU's Embedded Systems course (Spring 2020).

<div align="center">
<sub>Made by <a href="#">Mohammad Asfour</a> · NYU Tandon</sub>
</div>
