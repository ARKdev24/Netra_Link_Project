# Netra_Link_Project
# Netra-Link: IoT-Enabled Assistive Goggle
### 🏆 3rd Prize Winner - IEEE SSCS Competition ('Helping People With Disability')

## 📖 Overview
Netra-Link is a smart wearable interface designed for individuals with severe motor disabilities (ALS, Paralysis). It translates voluntary eye-blink patterns into digital commands for **Home Automation** (lights, fans) and **Emergency Response**. 

Unlike standard eye-trackers, this system runs on low-cost hardware (ESP32) and includes a novel **"Unconsciousness Detection"** algorithm that triggers an alert if the user's eye remains closed for an abnormal duration (suggesting a faint or medical event).

## ✨ Key Features
- **Smart Blink Detection:** Distinguishes between:
  - *Natural Blinks* (< 400ms): Ignored.
  - *Command Blinks* (> 2000ms): Toggles appliances.
  - *SOS Triggers* (Double Blink): Sends immediate emergency alert.
- **Unconsciousness Monitor:** Automatically detects if the user is unresponsive (eye closed > 8 seconds) and alerts caregivers.
- **IoT Connectivity:** Uses **HTTP/REST API** to send push notifications via Pushbullet.
- **Menu System:** (Optional) Integration with 16x2 I2C LCD to visualize menu selection (Light -> Fan -> Speaker).

## 🛠️ Hardware Stack
- **Microcontroller:** ESP32 Dev Module (Wi-Fi enabled)
- **Sensors:** IR Obstacle Sensor (Active Low)
- **Actuators:** 3-Channel Relay Module (for appliances), Piezo Buzzer
- **Display:** 16x2 LCD (I2C)
- **Power:** 3.7V Li-Ion Battery

## 🔌 Pin Configuration
| Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **IR Sensor** | GPIO 15 | Input (Eye Status) |
| **Device 1 (Light)** | GPIO 18 | Output Relay |
| **Device 2 (Fan)** | GPIO 23 | Output Relay |
| **Device 3 (Speaker)** | GPIO 19 | Output Relay |
| **SDA (LCD)** | GPIO 21 | I2C Data |
| **SCL (LCD)** | GPIO 22 | I2C Clock |

## 💻 Software & Logic
The firmware is written in C++ (Arduino Framework).

### Core Algorithm (Simplified):
1. **State Machine:** Monitors `IR_PIN` for falling edges (eye closure).
2. **Timing Logic:** - Calculates `blinkDuration = millis() - blinkStart`.
   - Filters noise (blinks < 200ms).
3. **Control Logic:**
   - **Double Blink (High Speed):** Triggers `push_SOS("SOS Sent")`.
   - **Long Hold (> 2.5s):** Toggles current selected device (ON/OFF).
   - **Extended Hold (> 5.0s):** Cycles through device menu (Light -> Fan -> Speaker).
4. **Safety Net:** If `blinkDuration > 8000ms` (8 seconds), enters **Emergency Mode** and sends "User Unconscious" alerts to the cloud.

## 🚀 Usage / Setup
1. **Install Libraries:**
   - `WiFi.h` (Built-in)
   - `HTTPClient.h` (Built-in)
   - `LiquidCrystal_I2C` (by Frank de Brabander)
2. **Configure Credentials:**
   Update the following lines in `main.ino`:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* pass = "YOUR_WIFI_PASSWORD";
   const char* token = "YOUR_PUSHBULLET_API_TOKEN";
