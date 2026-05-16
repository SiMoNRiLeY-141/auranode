---
publishDate: 2026-05-15T00:00:00Z
title: AuraNode - Industry 4.0 Predictive Maintenance
excerpt: An Edge-AI IoT node that uses dynamic rhythm calibration and a touchless optical HMI to monitor machinery health.
image: auranode/cover.jpg
tags:
  - IoT
  - Edge-AI
  - Predictive Maintenance
  - ESP32
---

> AuraNode: Machine-agnostic predictive maintenance with zero-touch safety.

## Acknowledgements
Special thanks to the MYOSA Sensors Council for providing the core hardware modules that made this rapid prototyping possible.

## Overview
AuraNode is an Industry 4.0 predictive maintenance system designed to prevent catastrophic machinery failure. Instead of relying on static, hard-coded safety thresholds, AuraNode uses Edge-AI to mathematically learn the unique kinetic rhythm of any machine it is attached to. If a bearing degrades or a shaft misaligns, the system detects the anomaly, triggers a localized industrial latching alarm, and pushes a diagnostic report to a supervisor via Bluetooth Classic.

**Key features:**
* Dynamic Baseline Calibration (Machine Agnostic)
* Zero-Drop Data Validation Gating (Hardware Glitch Filtering)
* Touchless Optical HMI (Shadow Mute)
* 10-Second Industrial Safety Lockout
* Real-time Bluetooth Telemetry

## Demo/Examples

### **Images**
<p align="center">
<img src="/assets/images/auranode/serial-output.jpg" width="800"><br/>
<i>AuraNode pushing real-time Bluetooth diagnostics to a remote supervisor during a simulated machine failure.</i>
</p>

### **Videos**
<video controls width="100%">
<source src="/demo.mp4" type="video/mp4">
</video>

## Features (Detailed)

### **1. Dynamic Rhythm Calibration**
During the first 5 seconds of boot-up, AuraNode samples the MPU6050 vibration sensor hundreds of times to calculate a 3D kinetic baseline vector. It then monitors for deviations from this baseline, allowing it to adapt to any motor size or RPM without reprogramming.

### **2. Data Validation Gating**
To combat I²C bus instability and "ghost frames" (where sensors momentarily return 0.0 values), the ESP32 runs a data validation loop. It strictly ignores impossible physical readings, completely eliminating false-positive alarms.

### **3. Touchless Optical HMI**
Factory technicians often wear heavy, oil-stained PPE. AuraNode uses an APDS9960 Ambient Light Sensor as a touchless interface. To acknowledge an alarm, a technician simply casts a physical shadow over the sensor.

### **4. Safety Lockout & Bluetooth**
When a fault triggers, the system enters a strict 10-second lockout where the alarm cannot be bypassed. Simultaneously, it broadcasts a formatted diagnostic report (including exact vibration deviation and temperature) over Bluetooth to the Android Serial Terminal.

## Usage Instructions
To deploy AuraNode on a new machine:
1. Securely mount the ESP32 and sensor payload to the motor casing.
2. Power on the device while the motor is running normally. 
3. Wait 5 seconds for the `Base:` reading to lock in on the OLED screen.
4. Pair an Android device to `AuraNode_EdgeAI` via a Serial Bluetooth Terminal.
5. The system is now armed and monitoring.

## Tech Stack
* **C++ / Arduino IDE** (Core Logic)
* **ESP32** (Microcontroller & Bluetooth Classic)
* **MYOSA AccelAndGyro** (Kinetic Sensing)
* **MYOSA BarometricPressure** (Thermal Sensing)
* **MYOSA LightProximityAndGesture** (Optical Sensing/HMI)
* **Adafruit SSD1306** (OLED UI)

## Requirements / Installation
To compile this project, ensure you have the following libraries installed in your Arduino IDE:
```bash
Adafruit_GFX
Adafruit_SSD1306
BluetoothSerial