import paho.mqtt.client as mqtt
import json

def on_message(client, userdata, msg):

    data = json.loads(msg.payload.decode())

    print(f"Suhu: {data['temp']} °C | Kelembaban: {data['hum']} %")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
client.connect("192.168.24.251", 1883)
client.subscribe("iot/sensor")
client.on_message = on_message
client.loop_forever()
