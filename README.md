# Esp32--smart-parking-system
An ESP32-based smart parking solution that automates vehicle entry, slot allocation, and billing, reducing manual effort, parking congestion, and processing time.

#  Smart Parking Automation System

Wokwi Simulation link : https://wokwi.com/projects/466798130973985793

This project is a smart parking automation system built using an **ESP32** and simulated in **Wokwi**. It was developed to understand how different embedded components can work together to automate a parking system. The project uses RFID for vehicle authentication, ultrasonic sensors to detect parking slot occupancy, servo motors to control the entry and exit gates, and an OLED display to show parking information and billing.

## About the Project

Managing parking manually can lead to unnecessary waiting time, inefficient use of parking spaces, and human errors. This project aims to simplify the parking process by automating vehicle entry, parking slot monitoring, and exit.

When a vehicle arrives, the RFID card is verified before the entry gate opens. The system checks for available parking slots using ultrasonic sensors and updates the slot status on the OLED display. When the vehicle exits, the parking duration is calculated automatically, and the parking fee is displayed before opening the exit gate.

## Features

* RFID-based vehicle authentication
* Automatic entry and exit gate control
* Real-time parking slot monitoring
* Parking slot indication using LEDs
* OLED display for parking status and billing
* Automatic parking fee calculation based on parking duration
* Multi-slot parking management

## Technologies Used

* ESP32
* Arduino C++
* Wokwi Simulator
* RFID RC522
* HC-SR04 Ultrasonic Sensors
* Servo Motors
* SSD1306 OLED Display

## Project Files

* `smart_parking_system.ino` – Main program
* `diagram.json` – Wokwi circuit configuration
* `libraries.txt` – Required libraries
* `Smart_Parking_System_Report.pdf` – Project documentation

## What I Learned

Working on this project helped me understand:

* Programming an ESP32 using Arduino C++
* Interfacing RFID, ultrasonic sensors, servo motors, and an OLED display
* Combining multiple components into a single embedded system
* Building and testing embedded projects using the Wokwi simulator
* Designing a simple automation workflow for a real-world problem

## Future Improvements

Some improvements I plan to make in the future include:

* Adding Wi-Fi connectivity for remote monitoring
* Creating a web or mobile dashboard
* Storing parking records in a cloud database
* Supporting online parking reservations
* Adding license plate recognition

---

This project was developed as part of my learning journey in embedded systems and IoT. It helped me gain practical experience with ESP32 programming, sensor integration, and system automation using the Wokwi simulation platform.
