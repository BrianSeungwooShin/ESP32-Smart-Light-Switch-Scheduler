# ESP32-Smart-Light-Switch-Scheduler

An IoT home automation system built in C++ on an ESP32 microcontroller[cite: 1]. It features a self-hosted HTTP web server for local manual control and NTP-synchronized scheduled alarms to actuate a standard light switch via a servo motor[cite: 1].

## Features
* **Web Interface:** Responsive control UI hosted directly on the ESP32[cite: 1].
* **NTP Time Sync:** Connects to network time protocol servers over Wi-Fi for exact alarm timing[cite: 1].
* **Servo Actuation:** Physical switch actuation via PWM servo control[cite: 1].
* **Status Monitoring:** Real-time state tracking and Wi-Fi status indicator LEDs[cite: 1].

## Hardware & Wiring
* **Microcontroller:** ESP32 Dev Board[cite: 1]
* **Actuator:** Servo Motor connected to **GPIO 13**[cite: 1]
* **Status LEDs:** Green LED (**GPIO 4**), Red LED (**GPIO 15**)[cite: 1]

## Software Dependencies
* `ESP32Servo` Library[cite: 1]
* `WiFi.h` & `time.h` (Built-in ESP32 cores)[cite: 1]
