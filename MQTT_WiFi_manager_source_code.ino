#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <elapsedMillis.h> //load the library
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

const char* mqtt_server = "broker.mqtt-dashboard.com";

char pub_str[100];

float getRandom();

WiFiClient espClient;
PubSubClient client(espClient);


void callback(char* topic, byte* payload, unsigned int length)
{

Serial.print("Message arrived : ");
 Serial.print(topic);
 Serial.print(" : ");
 for (int i = 0; i < length; i++)
 {
 Serial.println((char)payload[i]);
 }
 if ((char)payload[0] == 'o' && (char)payload[1] == 'n')
 {
 digitalWrite(2, LOW);
 }
 else if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f' ) {
 digitalWrite(2, HIGH);
 }

}

void reconnect()
{

if(!client.connected()){
Serial.println("Attempting MQTT connection");
if(client.connect("clientId-c2ytKj0Axp"))
{
Serial.println("Connected");
client.publish("ReadRandom","Connected!");
client.subscribe("ReadLight");
Serial.print("subscribed!");
}
else
{
Serial.print("Failed, rc = ");
Serial.print(client.state());
Serial.println("Waiting for 5 seconds to try again");
delay(5000);
 }
 }
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void setup() {
  
Serial.begin(115200);
 pinMode(2, OUTPUT);
//Spiffs reading and saving External inputs from portal
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
         
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
 
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here



   /*   
Erase all previously saved Infotmations 

if (digitalRead(13) == HIGH) {    
    wifiManager.resetSettings();
  }
*/

wifiManager.autoConnect("Tunelark");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");


  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
   
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }


 client.setServer(mqtt_server, 1883);
 client.setCallback(callback);
 reconnect();
}

void loop() {

  if(!client.connected())
{
reconnect();
Serial.print("disconnected");
}
float tmp = getRandom();
//sprintf(pub_str,"%f", tmp);
dtostrf(tmp,2,6,pub_str);
Serial.print(pub_str);
Serial.println(tmp);
client.publish("ReadRandom",pub_str);
delay(1000);

client.loop();
}

float getRandom() {
 float val = analogRead(A0);
 float c = val * 285.0 / 1024.0;
 return c;
  // put your main code here, to run repeatedly:

}
