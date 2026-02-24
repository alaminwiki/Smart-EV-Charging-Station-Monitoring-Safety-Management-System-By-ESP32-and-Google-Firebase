import firebase_admin
from firebase_admin import credentials, db
import time
import os

# --- CONFIGURATION ---
CH_RATE = 0.20  # Price per minute of charging
STATIONS = ["ESP32_01", "ESP32_02", "ESP32_03", "ESP32_04"]

if not firebase_admin._apps:
    cred = credentials.Certificate("serviceAccountKey.json")
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://bd-ev-ch-default-rtdb.asia-southeast1.firebasedatabase.app'
    })

# Master structure for the Manager (Station-level totals)
manager_stats = {
    st: {"active_cars": 0, "total_sessions": 0, "revenue": 0.0} for st in STATIONS
}

def update_manager_data(event):
    """Refreshes totals whenever Firebase updates"""
    root_data = db.reference('devices').get()
    if not root_data: return

    for st_id in STATIONS:
        if st_id in root_data:
            st_data = root_data[st_id]
            charging_count = 0
            for i in range(1, 7):
                if st_data.get(f'charging{i}', False):
                    charging_count += 1
            
            manager_stats[st_id]["active_cars"] = charging_count
            # Note: Cumulative revenue is tracked by the main script's CSV or 
            # can be pulled from a 'totals' node in Firebase if you implement one.

def display_manager_dashboard():
    os.system('cls' if os.name == 'nt' else 'clear')
    print("=" * 70)
    print(f"{'EV NETWORK MANAGER SUMMARY':^70}")
    print("=" * 70)
    print(f"{'Station ID':<15} | {'Active Chargers':<18} | {'Status':<15}")
    print("-" * 70)
    
    grand_total_active = 0
    for st_id, stats in manager_stats.items():
        grand_total_active += stats["active_cars"]
        status = "🟢 ONLINE" if stats["active_cars"] >= 0 else "🔴 OFFLINE"
        
        print(f"{st_id:<15} | {stats['active_cars']:<18} | {status:<15}")
    
    print("-" * 70)
    print(f"TOTAL CARS CHARGING IN NETWORK: {grand_total_active}")
    print(f"ESTIMATED NETWORK LOAD: {grand_total_active * 7.4} kW") # Assuming 7.4kW per car
    print("=" * 70)
    print("Press Ctrl+C to switch back or exit.")

db.reference('devices').listen(update_manager_data)

try:
    while True:
        display_manager_dashboard()
        time.sleep(5) # Manager view doesn't need to refresh as fast
except KeyboardInterrupt:
    print("\nExiting Manager View...")