#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal    //Local WebServer used to serve the configuration portal
#include <ESP8266mDNS.h>
#include <DHT.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


int LED = D1;

float humidity, temp_f;  // Values read from sensor

#define 


#define DHTTYPE DHT11
#define DHTPIN  2

//******************************IBM Watson Connection Data ****************************
#define ORG "jhlp4o"
#define DEVICE_TYPE "WemosD1mini"
#define DEVICE_ID "ESP82661"
#define TOKEN "ESP82661"
//******************************IBM Watson Connection Data ****************************

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char publishTopic[] = "iot-2/evt/status/fmt/json";
const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";

void callback(char* topic, byte* payload, unsigned int payloadLength);
//--*


WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 6000; // 1 min
long lastPublishMillis;


Ticker ticker;

ESP8266WebServer webserver ( 8080 ); //Web server
DHT dht(DHTPIN, DHTTYPE); 


void tick()
{
  //toggle state
  int state = digitalRead(LED);  // get the current state of GPIO1 pin
   digitalWrite(LED, HIGH);    // set pin to the opposite state
}


void setup() {

Serial.begin(115200);

digitalWrite(D1, HIGH);

  Serial.println();

  WiFi.begin("SLT FIBRE 221_EXT", "0912242284");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

 //g_wifiManager.autoConnect("ESP8266_1_WS", "ESP82661");
//g_wifiManager.autoConnect("IBM_WATSON_TEMP");
 

  // start ticker with 0.5 because we start in AP mode and try to connect
  //ticker.attach(0.6, tick);

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  //g_wifiManager.setAPCallback(configModeCallback);


/*if (!g_wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    delay(10000);
    ESP.reset();
    
  } */
  


 // Serial.println("connected to AP)");
//  ticker.detach();


/* if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  } */
  
  webserver.on("/", handleRoot);
  webserver.on("/inline", []() {
    webserver.send(200, "text/plain", "this works as well");
  });


 webserver.begin();
  Serial.println("HTTP server started in 8080");

  mqttConnect();
  digitalWrite(LED, HIGH);
 initManagedDevice();

 

}

void loop() {

  webserver.handleClient(); 

if (millis() - lastPublishMillis > publishInterval) {

Serial.print("Reading Sensor Data \n");
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit

     if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      temp_f = 10;
      humidity = 10;
    }

   publishData(temp_f,humidity);
   lastPublishMillis = millis();
 }

 if (!client.loop()) {
    
   mqttConnect();
   initManagedDevice();
 }
 

}



void handleRoot() {
  
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  webserver.send(200, "text/html", temp);
}



void mqttConnect() {
 if (!!!client.connected()) {
   Serial.print("Reconnecting MQTT client to "); Serial.println(server);
   while (!!!client.connect(clientId, authMethod, token)) {
     Serial.print(".");
     delay(500);
   }
   Serial.println();
 }
}

void initManagedDevice() {
 if (client.subscribe("iotdm-1/response")) {
   Serial.println("subscribe to responses OK");
 } else {
   Serial.println("subscribe to responses FAILED");
 }

 if (client.subscribe(rebootTopic)) {
   Serial.println("subscribe to reboot OK");
 } else {
   Serial.println("subscribe to reboot FAILED");
 }

 if (client.subscribe("iotdm-1/device/update")) {
   Serial.println("subscribe to update OK");
 } else {
   Serial.println("subscribe to update FAILED");
 }

 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& d = root.createNestedObject("d");
 JsonObject& metadata = d.createNestedObject("metadata");
 metadata["publishInterval"] = publishInterval;
 JsonObject& supports = d.createNestedObject("supports");
 supports["deviceActions"] = true;

 char buff[300];
 root.printTo(buff, sizeof(buff));
 Serial.println("publishing device metadata:"); Serial.println(buff);
 if (client.publish(manageTopic, buff)) {
   Serial.println("device Publish ok");
 } else {
   Serial.print("device Publish failed:");
 }
}

void publishData(float temp,float humd) {
 String payload = "{\"d\":{\"temperature\":";
 payload += temp;
 payload += ",";
 payload += "\"humidity\":";
 payload += humd;
 payload += "}}";

 Serial.print("Sending payload: "); Serial.println(payload);

 if (client.publish(publishTopic, (char*) payload.c_str())) {
   Serial.println("Publish OK");
 } else {
   Serial.println("Publish FAILED");
 }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
 Serial.print("callback invoked for topic: "); Serial.println(topic);

 if (strcmp (responseTopic, topic) == 0) {
   return; // just print of response for now
 }

 if (strcmp (rebootTopic, topic) == 0) {
   Serial.println("Rebooting...");
   ESP.restart();
 }

 if (strcmp (updateTopic, topic) == 0) {
   handleUpdate(payload);
 }
}

void handleUpdate(byte* payload) {
  
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
   Serial.println("handleUpdate: payload parse FAILED");
   return;
 }
 Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();

 JsonObject& d = root["d"];
 JsonArray& fields = d["fields"];
 for (JsonArray::iterator it = fields.begin(); it != fields.end(); ++it) {
   JsonObject& field = *it;
   const char* fieldName = field["field"];
   if (strcmp (fieldName, "metadata") == 0) {
     JsonObject& fieldValue = field["value"];
     if (fieldValue.containsKey("publishInterval")) {
       publishInterval = fieldValue["publishInterval"];
       Serial.print("publishInterval:"); Serial.println(publishInterval);
     }
   }
 }
}
