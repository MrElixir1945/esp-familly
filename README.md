# ESP Family

Collection of ESP32 and Arduino projects for IoT, automation, and security testing.

## Projects

### Smart-Lamp
Control a lamp using **Telegram Bot** + **ESP32**. Includes firmware and Python bot.
- `Firmware.py` — ESP32 firmware code
- `bot.py` — Telegram bot backend
- `main.py` — main entry point
- `requirments.txt` — Python dependencies
- `.env.example` — environment config template

### BLE Ducy
Bluetooth Low Energy (BLE) keyboard injection tool for ESP32.
- `esp-bleducy-friendly.ino` — main BLE ducky sketch
- `rubber-ducy-asli/esp32-rubberducy.ino` — original rubber ducky via BLE

### Rubber Ducky
Classic USB HID keystroke injection for Arduino (Leonardo / Micro).
- `mpls.ino` — payload script
