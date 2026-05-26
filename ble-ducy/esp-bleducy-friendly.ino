/**
 * PROJECT: EXTROS V3 (Security Research PoC)
 * TARGET: ESP32 DevKit V1
 * AUTHOR: Mr. Elixir
 * PURPOSE: Bluetooth Low Energy (BLE) HID Injection Demonstration
 */

#include <WiFi.h>
#include <WebServer.h>
#include <BleKeyboard.h> 

// ==========================================
// CONFIGURATION
// ==========================================
const char* ap_ssid = "EXTROS";      
const char* ap_password = "password123";   

// Bluetooth Device Name (Stealth Mode)
BleKeyboard bleKeyboard("Generic Keyboard", "HP", 100);

WebServer server(80);
const int ledPin = 2;
bool isStressTestActive = false;

// ==========================================
// WEB DASHBOARD (Stored in Flash Memory)
// ==========================================
const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EXTROS V3 // C2</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;700&display=swap');
        
        body { 
            background-color: #0a0a0a; 
            color: #00ff41; 
            font-family: 'JetBrains Mono', monospace; 
            display: flex; 
            justify-content: center; 
            padding: 20px;
        }

        .container { width: 100%; max-width: 400px; display: flex; flex-direction: column; gap: 15px; }

        .header { border-bottom: 1px solid #333; padding-bottom: 10px; margin-bottom: 10px; }
        .title { font-size: 24px; font-weight: bold; }
        .subtitle { font-size: 12px; color: #666; }

        .status-box {
            background: #111; border: 1px solid #333; padding: 15px;
            text-align: center; border-radius: 4px;
        }
        
        .status-text { font-size: 18px; font-weight: bold; margin-bottom: 5px; }
        .connected { color: #00ff41; }
        .disconnected { color: #555; }
        .active { color: #ff3333; animation: pulse 1s infinite; }

        button {
            background: transparent; border: 1px solid #00ff41; color: #00ff41;
            padding: 15px; font-family: inherit; font-size: 16px; cursor: pointer;
            margin-top: 5px; transition: 0.2s;
        }
        button:hover { background: rgba(0, 255, 65, 0.1); }
        button:active { transform: scale(0.98); }

        .btn-stop { border-color: #ff3333; color: #ff3333; }
        .btn-stop:hover { background: rgba(255, 51, 51, 0.1); }

        .log-terminal {
            background: #000; border: 1px solid #333; height: 120px;
            padding: 10px; font-size: 11px; color: #888; overflow-y: auto;
        }

        @keyframes pulse { 0% {opacity:1;} 50% {opacity:0.6;} }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="title">EXTROS V3</div>
            <div class="subtitle">BLE HID INJECTION TOOL</div>
        </div>

        <div class="status-box">
            <div id="statusText" class="status-text disconnected">WAITING FOR TARGET...</div>
            <div id="subStatus" style="font-size: 12px; color: #555;">SCANNING BLUETOOTH</div>
        </div>

        <button onclick="sendCommand('/inject')">⚡ INJECT PAYLOAD (PoC)</button>
        <button class="btn-stop" onclick="sendCommand('/stop')">🛑 STOP ALL PROCESSES</button>

        <div class="log-terminal" id="console">
            <div>> SYSTEM READY...</div>
        </div>
    </div>

    <script>
        function log(msg) {
            const t = document.getElementById('console');
            const time = new Date().toLocaleTimeString().split(' ')[0];
            t.innerHTML += `<div>[${time}] ${msg}</div>`;
            t.scrollTop = t.scrollHeight;
        }

        function sendCommand(endpoint) {
            log("EXECUTING: " + endpoint);
            fetch(endpoint);
        }

        setInterval(() => {
            fetch('/status').then(r => r.json()).then(d => {
                const txt = document.getElementById('statusText');
                const sub = document.getElementById('subStatus');
                
                if(d.stress) {
                    txt.innerText = "⚠️ INJECTION ACTIVE";
                    txt.className = "status-text active";
                    sub.innerText = "SENDING KEYSTROKES...";
                } else if(d.conn) {
                    txt.innerText = "TARGET CONNECTED";
                    txt.className = "status-text connected";
                    sub.innerText = "READY TO DEPLOY";
                } else {
                    txt.innerText = "DISCONNECTED";
                    txt.className = "status-text disconnected";
                    sub.innerText = "WAITING FOR BLUETOOTH PAIRING";
                }
            });
        }, 1000);
    </script>
</body>
</html>
)=====";

// ==========================================
// LOGIC
// ==========================================

void typeText(String text) {
  for(int i = 0; i < text.length(); i++) {
    bleKeyboard.print(text[i]);
    delay(20); 
  }
}

void handleInject() {
  if(!bleKeyboard.isConnected()) return;
  digitalWrite(ledPin, HIGH); 
  
  // 1. Buka Spotlight/Search (Universal untuk iOS/Android/Windows)
  bleKeyboard.press(KEY_LEFT_GUI); 
  bleKeyboard.print(" "); 
  bleKeyboard.releaseAll();
  delay(500);

  // 2. Ketik Pesan Edukasi
  typeText("SECURITY ALERT: YOUR DEVICE IS VULNERABLE TO HID INJECTION");
  delay(1000);
  
  // 3. Masuk ke Notepad/Browser (Simple PoC)
  bleKeyboard.print(KEY_RETURN);
  
  server.send(200, "text/plain", "OK");
  digitalWrite(ledPin, LOW);
}

void handleStop() {
  isStressTestActive = false;
  digitalWrite(ledPin, LOW); 
  server.send(200, "text/plain", "STOPPED");
}

void handleStatus() {
  String json = "{\"conn\":" + String(bleKeyboard.isConnected()?"true":"false") + ",";
  json += "\"stress\":" + String(isStressTestActive?"true":"false") + "}";
  server.send(200, "application/json", json);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  
  bleKeyboard.begin();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  server.on("/", [](){ server.send(200, "text/html", index_html); });
  server.on("/inject", handleInject);
  server.on("/stop", handleStop);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
  
  // Fitur "Stress Test" (Versi aman: ada delay biar device gak hang permanen)
  if (isStressTestActive && bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    delay(100); // Delay safety biar gak crash total
  }
}