# 🚑 EMV Route Alert System

## Emergency Route Display System with Real-Time Directional Alerts

An IoT and Embedded Systems project designed to assist emergency vehicles such as ambulances, and fire brigades vehicles by providing real-time directional alerts at road intersections.

The system uses an ESP32-based transmitter installed in the emergency vehicle and an ESP8266-based receiver installed at a junction. Direction information is transmitted wirelessly using NRF24L01 modules and displayed on four LCD screens positioned around the intersection.

---

## 📌 Project Overview

Emergency vehicles often face delays due to traffic congestion and lack of awareness among road users. Drivers may hear a siren but cannot determine from which direction the vehicle is approaching.

The EMV Route Alert System solves this problem by:

* Detecting the emergency vehicle heading using a QMC5883L digital compass.
* Allowing the driver to select the intended route (Straight, Right, Left, or U-turn).
* Transmitting route information wirelessly using NRF24L01 RF modules.
* Displaying incoming and outgoing directions on four LCD displays placed at a junction.
* Providing audible alerts through a buzzer.

This helps road users identify the emergency vehicle's direction and clear the route more quickly.

---

## 🎯 Objectives

* Reduce emergency vehicle response time.
* Improve road safety.
* Provide real-time directional alerts.
* Develop a low-cost smart traffic assistance system.
* Support future smart city infrastructure.

---

## 🏗 System Architecture

### Transmitter Unit (Emergency Vehicle)

Components:

* ESP32 Dev Board
* NRF24L01 + PA + LNA Module
* QMC5883L Digital Compass
* 4 Direction Selection Buttons
* 4 Status LEDs
* Master Stop Button
* TP4056 Charging Module
* Step-Up Boost Converter
* 3.7V Li-ion Battery

Functions:

* Detect vehicle heading.
* Select intended route.
* Calculate incoming and outgoing direction.
* Transmit data wirelessly.

---

### Receiver Unit (Junction)

Components:

* ESP8266 NodeMCU
* NRF24L01 Receiver
* Active Buzzer
* 4 × 16×2 I2C LCD Displays

LCD Addresses:

| Display | I2C Address |
| ------- | ----------- |
| LCD 1   | 0x27        |
| LCD 2   | 0x26        |
| LCD 3   | 0x25        |
| LCD 4   | 0x24        |

Functions:

* Receive wireless route information.
* Display incoming and outgoing directions.
* Rotate directional arrows according to display orientation.
* Generate audible alerts.

---

## 📡 Communication Protocol

### Wireless Module

NRF24L01

### Frequency

2.4 GHz

### Packet Structure

```cpp
struct PacketData {
    int inAngle;
    int outAngle;
};
```

Where:

* `inAngle` = Current vehicle heading
* `outAngle` = Intended route direction

---

## 🔘 Route Selection

| Button        | Action            |
| ------------- | ----------------- |
| B1            | Straight          |
| B2            | Right Turn        |
| B3            | U-Turn            |
| B4            | Left Turn         |
| Master Button | Stop Transmission |

Direction Calculation:

| Action   | Formula        |
| -------- | -------------- |
| Straight | Heading + 0°   |
| Right    | Heading + 90°  |
| U-Turn   | Heading + 180° |
| Left     | Heading + 270° |

---

## 🧭 Junction Display System

The receiver uses four LCD displays positioned around a four-way junction.

```text
           LCD1
             ↑

LCD4 ← Junction → LCD2

             ↓
           LCD3
```

Each LCD automatically rotates directional arrows based on its orientation:

| LCD  | Rotation |
| ---- | -------- |
| LCD1 | 0°       |
| LCD2 | +90°     |
| LCD3 | +180°    |
| LCD4 | +270°    |

---

## 📺 LCD Information

Each display shows:

```text
EMV OUTGOING:
EMV INCOMING:
```

### EMV INCOMING

Direction from which the emergency vehicle is approaching.

### EMV OUTGOING

Direction in which the emergency vehicle intends to move after reaching the junction.

---

## 🔊 Alert System

When signal is received:

* LCD backlights turn ON.
* Direction arrows blink.
* Buzzer activates.

When signal is lost:

* LCD displays clear.
* Backlights turn OFF.
* Buzzer turns OFF.

Timeout: 2 Seconds

---

## ⚡ Power Supply

### Transmitter

```text
Li-ion Battery
      ↓
TP4056 Charger
      ↓
Boost Converter
      ↓
ESP32 + NRF24L01 + QMC5883L
```

### Receiver

```text
External 5V Supply
      ↓
ESP8266
      ↓
NRF24L01
      ↓
LCD Displays + Buzzer
```

---

## 🚀 Features

* Real-Time Direction Detection
* Wireless RF Communication
* Multi-Directional LCD Display Network
* Audible Warning System
* Rechargeable Battery Powered Transmitter
* Automatic Display Rotation
* Smart Junction Alert System
* Low-Cost Implementation

---

## 🎓 Applications

* Ambulances
* Fire Brigades
* Police Vehicles
* Smart Traffic Systems
* Smart Cities
* Emergency Response Networks

---

## 🔮 Future Enhancements

* GPS Tracking
* GSM Notification System
* Mobile Application Integration
* Cloud Monitoring Dashboard
* Traffic Signal Control Integration
* AI-Based Traffic Prediction

---

## 👨‍💻 Authors

**Balaji Dhakare**

Department of Electronics and Telecommunication Engineering
Sinhgad Institute Technology, Lonavala 

---

## 📜 License

This project is developed for educational and research purposes.
