import firebase_admin
from firebase_admin import credentials, db
import requests
import time

# --- CONFIGURATION ---
TELEGRAM_USER = "@alamindac"
DATABASE_URL = "https://bd-ev-ch-default-rtdb.asia-southeast1.firebasedatabase.app"

# Cooldown: Wait 60 seconds before calling again for the same station
CALL_COOLDOWN = 60 
last_call_time = {}

STATION_NAMES = {
    "ESP32_01": "Station One Basundhara",
    "ESP32_02": "Station Two Rampura",
    "ESP32_03": "Station Three Satarkul",
    "ESP32_04": "Station Four Natunbazar"
}

# --- FIREBASE SETUP ---
if not firebase_admin._apps:
    cred = credentials.Certificate("serviceAccountKey.json")
    firebase_admin.initialize_app(cred, {'databaseURL': DATABASE_URL})
print("✅ Connected to Firebase")

def trigger_telegram_call(station_id):
    global last_call_time
    current_time = time.time()
    
    if station_id not in last_call_time or (current_time - last_call_time[station_id]) > CALL_COOLDOWN:
        name = STATION_NAMES.get(station_id, station_id)
        message = f"Emergency Alert! Fire detected at {name}."
        url = f"http://api.callmebot.com/start.php?user={TELEGRAM_USER}&text={message}&lang=en-US-Standard-B&rpt=2"
        
        try:
            print(f"☎️ TRIGGERING CALL FOR: {name}")
            response = requests.get(url)
            if response.status_code == 200:
                last_call_time[station_id] = current_time
                print(f"📞 Call successful for {name}")
        except Exception as e:
            print(f"❌ Call Error: {e}")
    else:
        # Calculate remaining seconds for the cooldown
        remaining = int(CALL_COOLDOWN - (current_time - last_call_time[station_id]))
        print(f"⏳ Cooldown active for {station_id} ({remaining}s left)")

def fire_listener(event):
    # Log everything to the console so we can monitor
    print(f"LOG: Path={event.path} Data={event.data}")
    
    # CASE 1: The event path itself contains the station and field (e.g. /ESP32_01/flame_detected)
    if "flame_detected" in event.path and event.data == True:
        station_id = event.path.split('/')[1]
        trigger_telegram_call(station_id)
        
    # CASE 2: The event path is just the station (e.g. /ESP32_03) and the data is a dictionary
    elif isinstance(event.data, dict) and event.data.get("flame_detected") == True:
        station_id = event.path.strip('/')
        # Handle cases where path is just '/'
        if not station_id: 
             # If the whole 'devices' node was updated, loop through it
             for sid, values in event.data.items():
                 if isinstance(values, dict) and values.get("flame_detected") == True:
                     trigger_telegram_call(sid)
        else:
            trigger_telegram_call(station_id)

# Watch the 'devices' folder
db.reference('devices').listen(fire_listener)

print("🔥 MONITORING ACTIVE - SYSTEM STABILIZED")
while True:
    time.sleep(1)