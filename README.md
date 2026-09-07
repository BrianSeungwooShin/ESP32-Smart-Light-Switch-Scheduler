# ESP32-Smart-Light-Switch-Scheduler

An IoT home automation system built in C++ on an ESP32 microcontroller. It features a self-hosted HTTP web server for local manual control and NTP-synchronized scheduled alarms to actuate a standard light switch via a servo motor.

## Features
* **Web Interface:** Responsive control UI hosted directly on the ESP32.
* **NTP Time Sync:** Connects to network time protocol servers over Wi-Fi for exact alarm timing.
* **Servo Actuation:** Physical switch actuation via PWM servo control.
* **Status Monitoring:** Real-time state tracking and Wi-Fi status indicator LEDs.

## Hardware & Wiring
* **Microcontroller:** ESP32 Dev Board
* **Actuator:** Servo Motor connected to **GPIO 13**
* **Status LEDs:** Green LED (**GPIO 4**), Red LED (**GPIO 15**)

## Software Dependencies
* `ESP32Servo` Library
* `WiFi.h` & `time.h` (Built-in ESP32 cores)
