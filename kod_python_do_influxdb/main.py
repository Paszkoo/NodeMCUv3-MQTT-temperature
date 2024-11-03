import asyncio
import ssl
from gmqtt import Client as MQTTClient
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Konfiguracja MQTT
MQTT_USERNAME = "testowy"
MQTT_PASSWORD = "test"
MQTT_BROKER = "a9aa313a.ala.eu-central-1.emqxsl.com"
MQTT_PORT = 8883
MQTT_TOPIC = "test_topic"
CLIENT_ID = "mqtt_python_data_collector"

# Ścieżka do certyfikatu CA
CA_CERT = "C:\\Users\\Jakub\\Desktop\\emqxsl-ca.crt"  # Replace with the path to your CA certificate

# Konfiguracja InfluxDB 2.x
INFLUXDB_URL = "http://localhost:8086"
INFLUXDB_TOKEN = "vx9-n5W2ZxrCsOdsyL3R-hnKqBgLcaBgM0dNlyrqUZR5SN4_z3wklaqi3H4GsxyUMoaVvR-r-ZKSELXc2nZ0PA=="
INFLUXDB_ORG = "test"
INFLUXDB_BUCKET = "mqtt_data"

# Połączenie z InfluxDB
influx_client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_TOKEN)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

# Asynchroniczna obsługa wiadomości MQTT
async def on_message(client, topic, payload, qos, properties):
    try:
        message = payload.decode()
        # Check if the message is a valid number
        temperature = float(message) if message.replace('.', '', 1).isdigit() else None
        if temperature is not None:
            print(f"Received temperature: {temperature}")
            # Zapis do InfluxDB
            point = Point("test_topic").field("value", temperature)
            write_api.write(bucket=INFLUXDB_BUCKET, org=INFLUXDB_ORG, record=point, write_precision=WritePrecision.NS)
        else:
            print(f"Ignored non-numeric message: '{message}'")
    except ValueError as e:
        print(f"Error processing message: {e}")

async def main():
    # Konfiguracja klienta MQTT
    client = MQTTClient(CLIENT_ID)
    client.set_auth_credentials(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_message = on_message

    # Ustawienia SSL/TLS dla połączenia MQTT z certyfikatem CA
    ssl_context = ssl.create_default_context()
    ssl_context.verify_mode = ssl.CERT_REQUIRED
    ssl_context.load_verify_locations(CA_CERT)  # Wczytaj certyfikat CA

    try:
        # Połączenie z brokerem MQTT
        await client.connect(MQTT_BROKER, port=MQTT_PORT, ssl=ssl_context)
        print("Connected to MQTT broker")
    except Exception as e:
        print(f"Connection failed: {e}")
        return  # Exit if connection fails

    # Subskrypcja tematu
    client.subscribe(MQTT_TOPIC)

    # Uruchomienie klienta w pętli
    await asyncio.Future()  # Keeps the program running


if __name__ == '__main__':
    asyncio.run(main())
