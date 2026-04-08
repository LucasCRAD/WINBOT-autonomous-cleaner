# WINBOT Autonomous Window Cleaning System
> Arduino Nano · 2–3x L298N · 4x TT Gear Motors · Winch/Pulley

An autonomous window cleaning cart that travels a calculated distance across a rooftop, controlling both lateral movement and a WINBOT winch/pulley retraction system. Speed and travel duration are dynamically calculated from user-defined building parameters.

---

## Table of Contents
- [Hardware](#hardware)
- [Wiring](#wiring)
- [Power](#power)
- [Parameters](#parameters)
- [How It Works](#how-it-works)
- [Calibration](#calibration)
- [Serial Monitor Output](#serial-monitor-output)
- [Code History](#code-history)
- [Future Improvements](#future-improvements)

---

## Hardware

| Component | Quantity | Notes |
|---|---|---|
| Arduino Nano | 1 | CH340 clone works fine |
| L298N Motor Driver | 3 | 2x cart motors, 1x winch |
| TT Geared DC Motor | 4 | 3V–6V rated |
| DC Motor (winch) | 1 | For WINBOT tether pulley |
| 9V Battery | 1 | Powers entire system in parallel |
| Breadboard | 1 | For parallel power distribution |
| Jumper wires | — | Male-to-male and male-to-female |

---

## Wiring

### L298N #1 — Cart Motors (Left Side)
| L298N Pin | Nano Pin |
|---|---|
| ENA1 | D5 (PWM ~) |
| IN1 | D6 |
| IN2 | D7 |
| ENB1 | D3 (PWM ~) |
| IN3 | D2 |
| IN4 | D4 |

### L298N #2 — Cart Motors (Right Side)
| L298N Pin | Nano Pin |
|---|---|
| ENA2 | D9 (PWM ~) |
| IN5 | D8 |
| IN6 | D10 |
| ENB2 | D11 (PWM ~) |
| IN7 | D12 |
| IN8 | D13 |

### L298N #3 — Winch Motor
| L298N Pin | Nano Pin |
|---|---|
| ENA3 | A0 |
| IN9 | A1 |
| IN10 | A2 |

> **Note:** Remove the ENA/ENB jumper caps from all L298N boards — these pins are controlled by the Nano via PWM.

---

## Power

All components are powered **in parallel** from a single 9V battery:

```
9V battery (+) ──┬──→ L298N #1 +12V terminal
                 ├──→ L298N #2 +12V terminal
                 ├──→ L298N #3 +12V terminal
                 └──→ Nano VIN pin

9V battery (-) ──┬──→ L298N #1 GND
                 ├──→ L298N #2 GND
                 ├──→ L298N #3 GND
                 └──→ Nano GND
```

> Use the breadboard power rails (+/-) to distribute from the battery to all components cleanly.

**Why parallel and not series?**
Each component needs the full 9V independently. Series would split the voltage. The Nano's onboard regulator steps 9V down to 5V internally. The L298N drops ~2V internally, so TT motors see approximately 7V — acceptable for short operation bursts.

---

## Parameters

At the top of the `.ino` file, edit these four values before uploading:

```cpp
const float COLUMN_SPAN_CM         = 50.0;  // width between vertical columns (cm)
const float ROOF_LENGTH_CM         = 200.0; // total horizontal travel distance (cm)
const float CALIBRATION_CM_PER_SEC = 16.6;  // cart speed at PWM 255 (measure this!)
const int   WINCH_SPEED            = 180;   // winch motor PWM speed (0–255)
```

| Parameter | Description |
|---|---|
| `COLUMN_SPAN_CM` | Distance between structural columns — used to calculate PWM speed |
| `ROOF_LENGTH_CM` | Full horizontal distance the cart must travel |
| `CALIBRATION_CM_PER_SEC` | Measured speed of your cart at full PWM — see Calibration below |
| `WINCH_SPEED` | How fast the winch pays out the WINBOT tether |

---

## How It Works

```
Power on
   │
   ▼
Calculate cart PWM from COLUMN_SPAN_CM
Calculate travel time from ROOF_LENGTH_CM
   │
   ▼
500ms motor-off pause (self-test)
   │
   ▼
All 4 cart motors move forward ──────────────────────────┐
Winch pays out tether simultaneously                     │
   │                                                     │
   ▼                                                     │
Serial monitor prints progress every 500ms               │
   │                                                     │
   ▼                                                     │
Travel time elapsed? ────── No ──────────────────────────┘
   │
   Yes
   ▼
All motors stop
Winch stops
Print "Cycle complete"
   │
   ▼
Press RESET to run again
```

---

## Calibration

The most important step before using the system on a real building:

1. Place the cart on a flat surface
2. Upload the code with `CALIBRATION_CM_PER_SEC = 16.6` (default)
3. Set `ROOF_LENGTH_CM = 100` and `COLUMN_SPAN_CM = 50` for a test run
4. Measure the actual distance the cart travels
5. Calculate your real speed:
```
actual cm travelled / (travelTimeMS / 1000) = your CALIBRATION_CM_PER_SEC
```
6. Update `CALIBRATION_CM_PER_SEC` with your measured value and re-upload

> Recalibrate any time you change battery, surface type, or motor load.

---

## Serial Monitor Output

Open Serial Monitor at **9600 baud** to see:

```
=== WINBOT Autonomous Cleaning System — Ready ===
-------------------------------------------------
Column span:       50.00 cm
Roof length:       200.00 cm
Calculated PWM:    77
Cart speed:        5.0 cm/s
Travel time:       40000 ms
Winch PWM:         180
-------------------------------------------------
Moving forward + winch paying out...
Progress: 10% — ~4.0 cm / 200.0 cm
Progress: 20% — ~8.0 cm / 200.0 cm
...
[STOP] Roof length reached — cycle complete.
Press RESET button to run again.
```

---

## Code History

| Version | Description |
|---|---|
| v1.0 | Basic single motor test, L293D, Arduino Uno |
| v2.0 | Rewritten for Arduino Nano + L298N, single motor |
| v3.0 | Dual L298N, 4x TT motors, fixed travel time |
| v4.0 | Triple L298N, winch motor, dynamic speed + time from building parameters |

---

## Future Improvements

- [ ] Encoder feedback for precise distance measurement (replace time-based travel)
- [ ] IR sensor for obstacle detection and emergency stop
- [ ] Wireless module (WiFi/Bluetooth) for remote monitoring
- [ ] Battery management system (BMS) for larger Li-ion pack
- [ ] Return journey with winch retraction sequence
- [ ] Multi-pass cleaning pattern (multiple column spans)
- [ ] Replace 9V battery with 24V Li-ion pack + BMS voltage regulation
