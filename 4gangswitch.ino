#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal    //Local WebServer used to serve the configuration portal
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
//----Define Switches(Relay) ----
#define switch1 D1
#define switch2 D2
#define switch3 D3
#define switch4 D4

//---Wifi Detals ----
#define wifiID "SLT FIBRE 221"
#define wifipassword "0912242284"

WiFiClient wifiClient; //wificlient
ESP8266WebServer webserver ( 8119 ); //Web server
ESP8266HTTPUpdateServer httpUpdater;


//-----GUI Vatibles ------
String txt_switch1 = "Switch 1";

void setup() {

  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(switch1, OUTPUT);


  boot();
  httpUpdater.setup(&webserver);
  MDNS.addService("http", "tcp", 80);

  otp_handle();



}

void loop() {

  digitalWrite(switch1, HIGH);
  digitalWrite(switch2, HIGH);
  digitalWrite(switch3, HIGH);
  digitalWrite(switch4, HIGH);

  webserver.handleClient();

}

void boot() {


  WiFi.begin(wifiID, wifipassword);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

   if (WiFi.status() == WL_CONNECTED) {
     
 WiFi.hostname("ESP4GA-upd");
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

   }




  Serial.println("Starting the WebServer");



  webserver.begin();
  Serial.println("HTTP server started in 8119");

  webserver.on("/", handleRoot);

}


void otp_handle() {

  Serial.println("Starting OTA Service");
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("myesp826612");


  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n OTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();


}

void handleRoot() {
  

webserver.send(200, "text/html", "<html>");  
webserver.send(200, "text/html", "<head><title>ESP4G</title></head>");  
//start_of_body
webserver.send(200, "text/html", "<body>");  
webserver.send(200,  "text/html", "<a href=\"/switch1\"><button>" + txt_switch1 + "></a>"); 
//end body here
webserver.send(200, "text/html", "</body></html>");  

}


void handle_response() {

    if (wifiClient) {

    while (wifiClient.connected()) {
      
      if (wifiClient.available()) { 
        
      char response =  wifiClient.read();
        
        
        } 
        
        
        }
      
      }
  
  }
