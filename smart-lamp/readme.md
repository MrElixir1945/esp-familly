# Smart-Lamp

Control a physical light switch remotely using ESP32 + Servo + Telegram Bot.

## How it works

ESP32 runs a small web server. The Python bot receives commands from Telegram, then sends HTTP requests to the ESP32. The ESP32 moves a servo to press the wall switch.

```
Telegram → Bot Server → ESP32 Web Server → Servo → Wall Switch
```

## Setup

### 1. Flash ESP32

Install MicroPython on ESP32, then upload `Firmware.py` as `main.py`:

```bash
pip install esptool adafruit-ampy
esptool.py --chip esp32 erase_flash
esptool.py --chip esp32 write_flash -z 0x1000 firmware.bin
ampy --port /dev/ttyUSB0 put Firmware.py main.py
```

Edit these values in `Firmware.py` before uploading:
```python
SSID = "YOUR_WIFI_NAME"
PASSWORD = "YOUR_WIFI_PASSWORD"
API_KEY = "YOUR_SECRET_API_KEY"
```

### 2. Setup Bot Server

```bash
pip install -r requirments.txt
cp .env.example .env
```

Fill in `.env`:
```
TELEGRAM_TOKEN=your_telegram_bot_token
ESP_IP=http://192.168.x.x
ESP_API_KEY=your_secret_api_key
```

> `ESP_API_KEY` must match `API_KEY` in `Firmware.py`.

### 3. Run

```bash
python bot.py
```

## Notes

- ESP32 and bot server must be on the **same WiFi network**
- Set ESP32 to a **static IP** in your router so it doesn't change
- Calibrate the servo angle in `Firmware.py` based on your physical setup
