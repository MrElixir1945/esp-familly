from machine import Pin, PWM, WDT, reset
import network, time, socket, gc

# --- BAGIAN PENTING: SESUAIKAN KREDENSIAL ---
SSID = "NAMA_WIFI_KAMU"
PASSWORD = "PASSWORD_WIFI_KAMU"
API_KEY = "12345_RAHASIA"
SERVO_PIN = 13 
# --------------------------------------------

current_state = False

def move_servo(angle):
    servo = PWM(Pin(SERVO_PIN), freq=50)
    duty_cycle = int(26 + (angle / 180) * (128 - 26))
    servo.duty(duty_cycle)
    time.sleep(0.5)
    servo.duty(0)
    servo.deinit()

def apply_servo():
    target_angle = 110 if current_state else 60
    move_servo(target_angle)

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        wlan.connect(SSID, PASSWORD)
        timeout = 20
        while not wlan.isconnected() and timeout > 0:
            time.sleep(1)
            timeout -= 1
    return wlan.isconnected()

if not connect_wifi():
    reset()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(('', 80))
s.listen(5)
s.settimeout(0.5)

wdt = WDT(timeout=30000)
apply_servo() 

while True:
    wdt.feed()
    
    if not network.WLAN(network.STA_IF).isconnected():
        if not connect_wifi():
            reset()

    try:
        conn, addr = s.accept()
        request = conn.recv(1024).decode()
        
        if API_KEY not in request:
            conn.close()
            continue

        response_body = ""
        if "/status" in request:
            response_body = '{"state": ' + ("true" if current_state else "false") + '}'
        elif "/toggle" in request:
            current_state = not current_state
            apply_servo()
            response_body = '{"state": ' + ("true" if current_state else "false") + '}'
            
        response = 'HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n' + response_body
        conn.send(response)
        conn.close()
    except OSError:
        pass
    gc.collect()