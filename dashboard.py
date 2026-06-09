import streamlit as st
import paho.mqtt.client as mqtt
import json
import pandas as pd
import time

data_list = []

def on_message(client, userdata, msg):
    global data_list
    data = json.loads(msg.payload.decode())
    data_list.append(data)

# Pakai VERSION1 agar warning Deprecation hilang
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
client.connect("192.168.24.251", 1883)
client.subscribe("iot/sensor")
client.on_message = on_message
client.loop_start()

st.title("📊 Dashboard IoT ESP32 (Windows 11)")

chart = st.empty()

while True:
    if len(data_list) > 0:
        df = pd.DataFrame(data_list)
        # Baris di bawah ini masuk ke dalam 'if'
        chart.line_chart(df)
    
    # time.sleep ini masuk ke dalam 'while', sejajar dengan 'if'
    time.sleep(2)