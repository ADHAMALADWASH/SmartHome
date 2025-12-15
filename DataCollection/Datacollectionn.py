import serial
import pandas as pd
from datetime import datetime
import time
import os
ser = serial.Serial("COM4", 9600)
filename = "Collecteddata.csv"
if not os.path.exists(filename):
    df = pd.DataFrame(columns=["soil_type", "state", "moisture_percent", "time", "pump_state"])
    df.to_csv(filename, index=False)

while True:
    try:
        line = ser.readline().decode().strip()
        if line == "":
            continue

        soil_type, state, moisture, pump = line.split(",")
        moisture = int(moisture)
        pump = int(pump)

        now = datetime.now().strftime("%H:%M:%S")

        new_data = pd.DataFrame([{
            "soil_type": soil_type,
            "state": state,
            "moisture_percent": moisture,
            "time": now,
            "pump_state": pump
        }])
        new_data.to_csv(filename, mode='a', header=False, index=False)
        print("Saved:", soil_type, state, moisture, now, pump)
    except Exception as e:
        print("Error:", e)
        time.sleep(1)
