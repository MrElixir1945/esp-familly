# BLE Ducy

Bluetooth Low Energy (BLE) keyboard injection tool for ESP32.

A proof-of-concept security testing tool that combines a **Web-based control panel** with **Bluetooth keyboard emulation**.

## How it works

The ESP32 creates a **Wi-Fi Access Point**. You connect to it from your browser to control the payload, while the ESP32 acts as a Bluetooth keyboard to the target device.

```
Attacker Browser → [ESP32 AP] — BLE Pairing —→ [Target Device]
                         ↓
                   Inject Keystrokes
```

## Files

- `esp-bleducy-friendly.ino` — main sketch with Web UI + BLE keyboard
- `rubber-ducy-asli/esp32-rubberducy.ino` — original BLE rubber ducky implementation

## Hardware

| Component | Detail |
|-----------|--------|
| Minimum | ESP32-S3 (recommended for best BLE performance) |
| Alternative | ESP32 DevKit V1 (DOIT) |
| Library | ESP32-BLE-Keyboard (by T-vK) |

## Setup

1. Install **Arduino IDE**
2. Install [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) library
3. Open the `.ino` file, select board, and upload
4. Connect to Wi-Fi: SSID `EXTROS-C2-POC`, password `password123`
5. Open browser to `http://192.168.4.1`
6. On target device, pair to **"Generic Keyboard"** via Bluetooth
7. Click **INJECT PAYLOAD** on the web dashboard

## Disclaimer

This project is for **educational purposes** and **security research only**. Only use on devices you own or have permission to test.
