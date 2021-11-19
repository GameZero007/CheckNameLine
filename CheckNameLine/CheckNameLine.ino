#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define WIFI_STA_NAME "NAPHAT_2.4G"
#define WIFI_STA_PASS "0918703498"
#define MQTT_SERVER   "mqtt.ezdrive.xyz"   // mqtt broker @ Digital System Engineering LAB (14-208) Siam university
#define MQTT_PORT     1883
#define MQTT_USERNAME "dsel0"         // Authentication  by user & password
#define MQTT_PASSWORD "piglet1234"
#define MQTT_NAME     "gamezz"    //MQTT_NAME OR Device_ID must has not duplicate in the system
#define TokenLine "irzjjYULx3C7ZqQ5HQbrzL1NGcBC9NRy00EWVjHuVkG"

// State Difinitions
#define ALL    1
#define ID    2

int  state  =0;
String txtfull;  
unsigned long  timebegin;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String timeStamp;
int splitT = formattedDate.indexOf("T");

unsigned long myTime; 
WiFiClient client;
PubSubClient mqtt(client);

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("callback in topic [" + topic_str + "]: " + payload_str);
  if(payload_str=="{\"155329\":\"ALL\"}")  {state=ALL;}
  else if(payload_str=="{\"155329\":\"6204000007\"}") {state=ID;}
}
void NotifyLine(String t) {
    WiFiClientSecure client;
    if (!client.connect("notify-api.line.me", 443)) {
      Serial.println("Connection failed");
      return;
    }
    String req = "";
    req += "POST /api/notify HTTP/1.1\r\n";
    req += "Host: notify-api.line.me\r\n";
    req += "Authorization: Bearer " + String(TokenLine) + "\r\n";
    req += "Cache-Control: no-cache\r\n";
    req += "User-Agent: ESP32\r\n";
    req += "Content-Type: application/x-www-form-urlencoded\r\n";
    req += "Content-Length: " + String(String("message=" + t).length()) + "\r\n";
    req += "\r\n";
    req += "message=" + t;
    Serial.println(req);
    client.print(req);
    delay(20);
    Serial.println("-------------");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
          break;
      }
    } //while
} // void Noti

void setup() {
  Serial.begin(115200);
  Serial.println( "\n");
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
  Serial.println("Connecting");
  timebegin = millis();
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  while(WiFi.status() != WL_CONNECTED && (millis()-timebegin<=15000) ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  timebegin=millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);   // connect to mqtt broker
  mqtt.setCallback(callback);               // set service for sub
}

void loop() { 
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedTime();
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length());
  if (mqtt.connected() == false) {
      Serial.print("MQTT connection... ");
         if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
              Serial.println("connected");
              mqtt.subscribe("/tawan/classcheckin/");
            } else {
                Serial.println("failed");
                delay(5000);
      }
  
    }else{
        switch (state) {
           case ALL: { 
            txtfull=("6204000007 "+ timeStamp);   
               NotifyLine(txtfull);
               Serial.print(txtfull);
               delay(1500);
             break;}
           
           case ID: { 
            txtfull="6204000007 "+ timeStamp;  
              NotifyLine(txtfull);
              Serial.print(txtfull);
              delay(1500);
             break;}
             }
            mqtt.loop();
           
    }
  }
