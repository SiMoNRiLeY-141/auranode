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

AuraNode was fundamentally a collaborative team effort, and its success is a testament to the incredible support system we had behind us.

First and foremost, our team extends its deepest gratitude to our faculty mentor, **Dr. Vinita Chellappan**. Her valuable guidance, technical insights, and unwavering support were instrumental in helping us navigate the complex engineering challenges of this project. We would also like to express our sincere appreciation to the other esteemed faculty members of our department for their continuous encouragement and foundational teachings.

We are incredibly grateful to the **MYOSA Sensors Council** for organizing this initiative and providing the core hardware payload that made this rapid prototyping possible. The provided components served as an excellent starting point for our Industry 4.0 vision.

Furthermore, we want to acknowledge the invaluable learning experience this project provided regarding real-world hardware integration. The unique challenges presented by our specific hardware kit—particularly the cloned silicon on our APDS9960 module and the I²C bus instabilities—pushed our team to move beyond standard educational libraries. It forced us to collaborate deeply, reverse-engineer the protocols, and engineer highly resilient, commercial-grade software bypasses, ultimately resulting in a much more robust final product.

Finally, a heartfelt thank you to our friends and peers for their late-night brainstorming sessions, willingness to help us test our hardware, and constant moral support throughout the duration of this competition.

## Overview

AuraNode is an advanced Industry 4.0 predictive maintenance and safety system designed to prevent catastrophic machinery failure in harsh industrial environments. Traditional factory monitoring relies on Programmable Logic Controllers (PLCs) with static, hard-coded safety thresholds that must be manually calibrated for every single machine. AuraNode disrupts this paradigm by utilizing local Edge-AI on the ESP32 to mathematically learn the unique kinetic and thermal profile of the specific machine it is attached to, adapting its own safety parameters dynamically.

**What problem it solves and Who it is for:**
Unplanned machinery downtime costs the manufacturing industry millions of dollars annually, and standard alarms are often prematurely silenced by operators before the core danger is registered. AuraNode is engineered specifically for industrial technicians on the factory floor and remote plant supervisors. It acts as an un-bypassable safety sentinel, ensuring that mechanical degradation (like bearing failure, friction overheating, or shaft misalignment) is caught instantly, while enforcing strict safety protocols that require physical inspection before the system can be reset.

**How it works & The Adaptive Hardware Pivot:**
During its initial boot-up phase, AuraNode establishes a customized "Baseline Rhythm" using multi-axis kinetic data. It continuously monitors for deviations from this baseline while actively filtering out I²C bus communication glitches using a custom Data Validation Gate. If a fault occurs, it triggers a localized industrial latching alarm, initiates a strict 10-second safety lockout (preventing immediate muting), and pushes a real-time diagnostic telemetry report to a supervisor via Bluetooth Classic.

Furthermore, because factory technicians often wear heavy, oil-stained PPE, traditional buttons or touchscreens are impractical. We originally intended to use an IR proximity sensor for gesture control, but discovered a fatal hardware fault (a dead IR emitter) in our kit's cloned silicon. In response, we engineered a robust software pivot: we repurposed the Ambient Light Sensor to serve dual, parallel functions. It now actively monitors for unauthorized machine casing breaches (detected as massive light spikes) while simultaneously serving as a Touchless Optical HMI—allowing technicians to acknowledge and mute alarms simply by casting a physical shadow over the sensor.

**Key features:**

- **Dynamic Baseline Calibration** (Machine Agnostic Operation)
- **Zero-Drop Data Validation Gating** (Hardware Glitch & Ghost Frame Filtering)
- **Adaptive Dual-Mode Optical HMI** (Tamper Detection & Touchless Shadow Mute)
- **10-Second Industrial Safety Lockout**
- **Real-time Bluetooth Telemetry & Push Notifications**

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

## Features & Engineering Pivots

During development, our team encountered several real-world hardware constraints. AuraNode's final architecture reflects agile engineering pivots to ensure industrial-grade reliability.

### **1. Machine-Agnostic Dynamic Calibration**

Instead of hard-coding static vibration thresholds (like standard PLCs), AuraNode mathematically learns the unique kinetic rhythm of any machine. During the first 5 seconds of boot-up, the system deep-flushes the I²C buffer and calculates the exact 3D kinetic energy vector ($\sqrt{x^2 + y^2 + z^2}$) across 100 valid samples to establish a custom baseline. It then monitors for deviations, allowing it to adapt to any motor size or RPM without reprogramming.

### **2. Adaptive Dual-Mode Optical HMI (Hardware Pivot)**

Our original proposal utilized the APDS9960's IR Proximity sensor for directional swipe gestures. Upon discovering our cloned silicon had a dead IR emitter, we executed a software pivot. We repurposed the Ambient Light Sensor to perform double-duty: it monitors for massive light spikes (detecting unauthorized casing breaches) and acts as a "Shadow Switch." Technicians simply cast a physical shadow over the sensor to touchlessly acknowledge and mute alarms without removing their PPE.

### **3. Zero-Drop Data Validation Gating**

I²C communication buses can drop frames due to electromagnetic interference, returning values of zero. We engineered an active Data Validation Gate to combat this. The ESP32 instantly discards physically impossible readings, entirely eliminating false-positive imbalance alarms caused by sensor "ghost frames."

### **4. Safety Lockout & Bluetooth Telemetry**

When a fault triggers, the system enters a strict 10-second lockout where the local alarm cannot be muted. Simultaneously, we pivoted from a Wi-Fi/MQTT architecture to **Bluetooth Classic**. This ensures 100% reliable telemetry push-notifications (detailing kinetic deviation, thermal state, and optical status) directly to a supervisor's Android terminal, completely immune to factory Wi-Fi outages.

## Usage Instructions

To deploy the AuraNode Edge-AI system on a new industrial asset:

1. **Mounting:** Securely mount the ESP32 and sensor payload to the exterior casing of the target motor or machine.
2. **Boot Sequence:** Power on the device while the machine is running under normal operational load.
3. **Calibration:** Wait exactly 5 seconds. Observe the OLED screen; the system will deep-flush the I²C buffer and lock in a custom `Base:` kinetic reading. Do not touch the machine during this phase.
4. **Connectivity:** On an Android device, open a Serial Bluetooth Terminal application and pair it to the device named `AuraNode_EdgeAI`.
5. **Operation:** The system is now armed. To test, induce an artificial imbalance. Once the alarm sounds and the 10-second lockout expires, cover the optical sensor with your hand to cast a shadow, which will acknowledge and mute the system.

## Tech Stack

- **C++ / Arduino IDE** (Core Embedded Logic)
- **ESP32** (Microcontroller & Bluetooth Classic Radio)
- **MYOSA AccelAndGyro** (MPU6050 Kinetic Sensing)
- **MYOSA BarometricPressure** (BMP180 Thermal Sensing)
- **MYOSA LightProximityAndGesture** (APDS9960 Optical Sensing & HMI)
- **Adafruit SSD1306** (OLED UI Dashboard)

## Requirements / Installation

To compile this project, ensure you have the following dependencies installed in your environment:

```bash
Adafruit_GFX
Adafruit_SSD1306
BluetoothSerial

```

## File Structure (Optional)

```text
/auranode
├─ code.ino
├─ demo.mp4
└─ README.md
```

## License (Optional)
This project is submitted under the MYOSA Hackathon rules.

---

## Contribution Notes (Optional)
As this is a hackathon submission, the core functionality is locked. However, discussions and feedback are welcome!
