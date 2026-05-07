# Hardware & Wiring

## Bill of Materials

| Qty | Part                                | Notes                              |
|-----|-------------------------------------|------------------------------------|
| 1   | Arduino Uno / Nano (ATmega328p)     | Any 5V AVR board works             |
| 1   | MPU6050 breakout board              | I²C, default address `0x68`        |
| 1   | LED (any color)                     | Can be replaced with a relay       |
| 1   | 220 Ω resistor                      | Current-limit for the LED          |
| —   | Jumper wires, breadboard            |                                    |

## Wiring Diagram

```
            ┌──────────────────┐
            │   ATmega328p     │
            │   (Arduino Uno)  │
            │                  │
            │  5V  ────────────┼──── VCC ──┐
            │  GND ────────────┼──── GND ──┤   ┌──────────────┐
            │  A4  (SDA) ──────┼──── SDA ──┼───│   MPU6050    │
            │  A5  (SCL) ──────┼──── SCL ──┼───│ Accelerometer│
            │                  │           └───└──────────────┘
            │  D13 ──[220Ω]───┼──┤▶├── GND   (LED = "unlocked" signal)
            └──────────────────┘
```

## Pin Map

| MPU6050 Pin | Arduino Pin |
|-------------|-------------|
| VCC         | 5V          |
| GND         | GND         |
| SDA         | A4          |
| SCL         | A5          |
| INT         | (unused)    |

| Output | Arduino Pin | Notes                 |
|--------|-------------|-----------------------|
| LED +  | D13         | through 220 Ω resistor |
| LED −  | GND         |                       |

> Replace the LED with a relay module on the same pin to drive a real
> door strike, solenoid, or any other resource.

## How the Math Works

The MPU6050 returns linear acceleration on three axes in m/s². At rest
on a flat surface, gravity is read as `(x≈0, y≈0, z≈9.81)`. A flick of
the wrist briefly pushes one axis past a threshold:

| Gesture   | Condition                                |
|-----------|------------------------------------------|
| Right     | `x < -4`                                 |
| Left      | `x >  4`                                 |
| Up        | `z >  13`                                |
| Down      | `z <  7`                                 |
| Forward   | `y < -3` and `\|x\| < 2`                 |
| Backward  | `y >  3` and `\|x\| < 2`                 |

A 500 ms debounce after each detection guarantees a single physical
flick maps to exactly one symbol in the sequence.
