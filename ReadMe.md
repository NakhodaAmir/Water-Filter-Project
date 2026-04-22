# Water Filter
> An Arduino-based embedded system built into a custom filtration housing. It utilizes a finite state machine to automate a triple-tank water processing setup, using ultrasonic and pH sensors to optimize fluid control, detect hardware faults, and display live telemetry.

## 🏆 Showcase
<img width="32%" alt="image" src="https://github.com/user-attachments/assets/44c24e36-8283-4b45-af8b-757991b0c455" />
<img width="32%" alt="image" src="https://github.com/user-attachments/assets/6e574491-980b-40e1-8031-d634050e8bef" />
<img width="32%" alt="image" src="https://github.com/user-attachments/assets/ee7ec111-1696-4d88-97e8-4430d696efb2" />

## 📖 Table of Contents
* [Features](#-features)
* [Quick Start](#-quick-start)
* [Architecture](#-architecture)
* [Project Structure](#-project-structure)
* [API Refrence](#-api-reference)

## ✨ Features
| Features | Description |
| -------- | ----------- |
| LCD Display | <ul><li>Displays Filtered Water/Output Tank Capacity</li><li>Displays PH levels of Intermediate/Output Tank</li><li>Displays Tank States</li><li>Displays Fault States</li></ul> |
| Fault Detection | <ul><li>When too much water is in the Filtered Water/Output Tank</li><li>PH level in Output Tank is greater than 9ph or less than 4p</li><li>When there is no water in the Filtered Water Tank</li><\ul> |

## 🚀 Quick Start
### Prerequisite
- Custom built water filter
### Install
Clone the repo to your local directory and upload the folder to the Arduino
```bash
git clone https://github.com/NakhodaAmir/Water-Filter-Project.git
```
### Run
Pour water

## ⚙️ Architecture
- The system has 3 tanks that contain water:
  - Intermediate Tank (Tank that hold the inputed water before the water passes through the filter)
  - Filtered Water Tank (Tank that hold the water that passed through the filter)
  - Output Tank (Tank that contains the filtered water that was pumped from the Filtered Water Tank)

<img width="736" height="385" alt="image" src="https://github.com/user-attachments/assets/98063f89-e372-4533-9cd9-2ff09e964b16" />

### Tank States
0% == Full;100% == Empty
| Tank State | Filtered Water Tank Capacity | Output Tank Capacity| Extra Conditions |
|-|-|-|-|
| IDLE | 100% | 100% | N.A. |
| FILLING | > 0% | N.A. | N.A. |
| READY | N.A. | > 0% | N. A. |
| DISPENSE | N.A. | > 0% | if Tank State is READY and dispense button is held |
| FAULT | N.A | N.A. | when any of the faults are true |

### Pump States
| Pump State | Condition |
|-|-|
| ON | when the filtered water tank has water |
| OFF | when the filtered water tank has no water or Tank State is FAULT |

### Fault States
| Fault State | Condition |
|-|-|
| NO_FAULT | when system is normal |
| OVERFLOW | when anytank is overflowing |
| INPUT_EMPTY | when there is no water for the pump to suck |
| ACIDIC | when the output tank ph is less than 4ph |
| BASIC | when the output tank ph is greater than 9ph |

## 📂 Project Structure
```text
main/
├──...
├── sketch.ino     # Arduino source code
├──...
└── README.md
```

## 📦 API Reference
### [class_name](link)
> **Purpose:** Controls a triple-tank automated water pumping and filtration system. It uses a finite state machine to manage a pump and a spigot based on water levels (via ultrasonic sensors) and water quality (via pH sensors), while actively monitoring for hardware faults and displaying real-time telemetry on an LCD.
* **Key Methods:**
    * `setup() -> void`: Initializes all hardware components (LCD, pins for sensors and actuators) and defaults the system to a safe IDLE state with hardware explicitly turned off.
    * `loop() -> void`: The core execution cycle. It handles four main tasks: reading sensors/debouncing inputs, evaluating state machine logic, checking global safety fault triggers, and updating the LCD UI.
    * `getDist(int, int) -> float`: Pulses the ultrasonic sensor, takes multiple samples, and returns a filtered raw distance reading.
    * `formatDist1(float) -> float`: Converts the raw distance from Tank 1 into a constrained percentage representing the water capacity (0% to 100%).
    * `formatDist2(float) -> float`: Converts the raw distance from Tank 2 into a constrained percentage representing the water capacity (0% to 100%).
    * `readPHNoSample(int) -> float`: Reads the analog voltage from the specified pH sensor and converts it to a baseline pH scale reading.
    * `tankStateToString(States) -> const char*`: Helper function that translates the tankState enum into a readable string for the LCD display.
    * `faultStateToString(Faults) -> const char*`: Helper function that translates the faultState enum into a readable string for the LCD display in the event of an error.
* **States:**
    * `tankState (enum States)`: The primary operational mode of the machine (IDLE, FILLING, READY, DISPENSE, FAULT, CLEAN).
    * `faultState (enum Faults)`: Tracks specific error conditions that trigger a system halt (NO_FAULT, OVERFLOW, INPUT_EMPTY, ACIDIC, BASIC, IN_SENSOR, OUT_SENSOR).
    * `pumpRunning (boolean)`: Tracks if the pump transferring water from Tank 1 to Tank 2 is actively powered.
    * `dispensing (boolean)`: Tracks if the spigot on Tank 2 is actively open.
    * `d1 and d2 (float)`: The calculated real-time capacity percentages of the input and output tanks.
    * `ph1 and ph2 (float)`: The calculated real-time pH readings for both tanks.
    * `right (boolean)`: A UI toggle state used to swap the LCD screen's focus between Tank 1 (Left) and Tank 2 (Right).
    * `currentButtonState / currentDisplayButtonState (boolean)`: Edge-detection states used to debounce user button presses for the spigot and display toggle.
