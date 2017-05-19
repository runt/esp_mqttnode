#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>

//#include <Wire.h>
#include <PubSubClient.h>
//#include <Adafruit_HDC1000.h>
#define mqtt_server "nr.runtici.cz"
#define mqtt_user "pepa"
#define mqtt_password "at90s2313"
#define NODEID  "esp_chodba"
#define NODETOPIC  "espnode/" NODEID

//#define humidity_topic "sensor/humidity"
//#define temperature_topic "sensor/temperature"
//#define switch_topic "sensor/switchuser"
#define HEARTBEAT_AFTER 60000

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wifiManager;


void setup() {
  Serial.begin(115200);
  //wifiManager.resetSettings();
  wifiManager.autoConnect();


  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("End OTA");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();


  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  Serial.print("MQTT server set");
  client.setServer(mqtt_server, 1883);

  // Set SDA and SDL ports
  //Wire.begin(2, 14);

  pinMode(16, OUTPUT);
  digitalWrite(16,LOW);
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect(NODEID, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      char topic[32];
      sprintf(topic,"%s/mqttreconnected",NODETOPIC);
      client.publish(topic,WiFi.localIP().toString().c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void r_publish(const char *topic,const char *value){
  char t[64];
  sprintf(t,"%s/%s",NODETOPIC,topic);
  client.publish(topic,value);
}

long lastMsg = 0;
char str_ip[24];

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  else{
    client.loop();
  }

  long now = millis();
  if (now - lastMsg > HEARTBEAT_AFTER) {
    lastMsg = now;
    r_publish("heartbeat",String(now).c_str());
    r_publish("ipaddress",WiFi.localIP().toString().c_str());
    /*
    char topic[64];
    sprintf(topic,"%s/heartbeat",NODETOPIC);
    client.publish(topic,String(now).c_str());
    sprintf(topic,"%s/ipaddress",NODETOPIC);
    client.publish(topic,WiFi.localIP().toString().c_str());
    */
  }
}
