#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// libraries
#include <GSM.h>

// PIN Number
#define PINNUMBER ""

// APN data
#define GPRS_APN       "internet" // replace your GPRS APN
#define GPRS_LOGIN     ""    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password

// MQTT Broker
#define MQTT_TOPIC "/jacek"
#define MQTT_CLIENTID "jacek"
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_USER ""
#define MQTT_PASSWORD ""
//#define MQTT_SERVER "ParkingInfo.azure-devices.net"
//#define MQTT_USER "ParkingInfo.azure-devices.net/"
//#define MQTT_PASSWORD ""

#define MAX_LENGTH 10

// initialize the library instance
GSMClient gsmClient;
GPRS gprs;
GSM gsmAccess;

PubSubClient client(gsmClient);

int lastValue = -1;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char playloadAsChars[MAX_LENGTH];
  memset(playloadAsChars,0,MAX_LENGTH);
  if(length >= MAX_LENGTH) {
    Serial.println("Too long string");
  } else {
    memcpy(playloadAsChars,payload,length);
    String payloadAsString = String(playloadAsChars);
    Serial.println(payloadAsString);
    int newValue = payloadAsString.toInt();
    if(lastValue != newValue) {
      Serial.print("Changed: ");
      Serial.println(newValue);
      lastValue = newValue;
    } else {
      Serial.print("Not changed: ");
      Serial.println(newValue);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}

void setup()
{
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting mqtt client.");
  // connection state
  boolean notConnected = true;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (notConnected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("connecting...");

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
