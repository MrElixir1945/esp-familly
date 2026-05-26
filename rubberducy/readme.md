# Rubber Ducky

USB HID keystroke injection payload for Arduino.

## Hardware

| Component | Detail |
|-----------|--------|
| Minimum | ESP32-S3 |
| Alternative | Arduino Leonardo, Arduino Micro, ESP32 (with USB-OTG) |

ESP32-S3 is the minimal ESP32 variant that supports native USB HID. Regular ESP32 (ESP-WROOM-32) does not have built-in USB-OTG — you would need an additional USB host shield.

## Files

- `mpls.ino` — payload script

## Usage

1. Upload `mpls.ino` to your board using Arduino IDE
2. Plug the board into the target computer via USB
3. The board will appear as a USB keyboard and execute the keystroke payload

## Disclaimer

For **educational purposes only**. Only use on devices you own or have permission to test.
