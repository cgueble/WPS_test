// Test for ESP8266 WPS connection.

#include <ESP8266WiFi.h>
#include <C:\Users\chris\Documents\Arduino\libraries\ArduinoMqttClient-master\src\ArduinoMqttClient.h>
//#include <C:\Users\chris\Documents\Arduino\libraries_Perso\Codes_CGU.h>
bool SERIAL_PORT_LOG_ENABLE = true;
bool wifiConnected = false;
String connectedToWifiName = "";
WiFiClient client;
MqttClient mqttClient(client);
String MqttPayload ="";
bool mqttRetained = false; 
int mqttQos = 1;
bool dup = false;
const char mqttServer[] = "mqtt.gueble.fr";//Adresse IP du Broker Mqtt
const int mqttPort = 8883; //port utilisé par le Broker 
const char willTopic[] = "esp/WPS/will";
const char inTopic[]   = "esp/WPS/in";
const char outTopic[]  = "esp/WPS";
const char clientId[] = "ESP01_WPS_search";
const char mqttUsername[] = "cgu";
const char mqttPassword[] = "cgu";

const int httpsPort = 443;
double BatteryVoltage = 0;
long RSSI = 0;


void setup() {
  delay(3000);//delai to get serial begin opperationnal
  Serial.begin(115200);
  while(!Serial){
    }//waiting for Serial communication to be opperationnal
  Serial.println("");
  Serial.println("setup ...");
   
  
  WiFi.mode(WIFI_STA); // WPS works in STA (Station mode) only.
  scan_Wifi();// Scan Wifi for info only
  Serial.println("setup Watch Dog to 2min");
  
  }//End setup()

void loop() {
  Serial.println("Start loop ...");
  Find_WPS_Wifi();//Si pas de Wifi connecté, on reste bloqué là en Scan WPS jusqu'a ce qu'une connexion Wifi se fasse
  setup_mqtt();
  //mqttClient.poll();
  delay(1000);
  SendWifiData();//Send Wifi Data of WPS found to ... email, SD card, MQTT .....
  delay(5000);
  disableWifi();
  Serial.println("End loop ... Delay 2 minutes");
  delay(10*60*1000);
  setup();
  //ESP.deepSleep(10);
  }//End loop()

void(* resetFunc) (void) = 0;
void disableWifi(){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  }
void SendWifiData(){
  mqtt_publish_String(outTopic,MqttPayload);
  }//End SendWifiData()
void scan_Wifi() {
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
  uint8_t *bssid;
  int32_t channel;
  bool hidden;
  int scanResult;
  Serial.println(F("Starting WiFi scan..."));
  scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
  if (scanResult == 0) {
    Serial.println(F("No networks found"));
  } else if (scanResult > 0) {
    Serial.printf(PSTR("%d networks found:\n"), scanResult);

    // Print unsorted scan results
    for (int8_t i = 0; i < scanResult; i++) {
      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);

      // get extra info
      const bss_info *bssInfo = WiFi.getScanInfoByIndex(i);
      String phyMode;
      const char *wps = "";
      if (bssInfo) {
        phyMode.reserve(12);
        phyMode = F("802.11");
        String slash;
        if (bssInfo->phy_11b) {
          phyMode += 'b';
          slash = '/';
        }
        if (bssInfo->phy_11g) {
          phyMode += slash + 'g';
          slash = '/';
        }
        if (bssInfo->phy_11n) {
          phyMode += slash + 'n';
        }
        if (bssInfo->wps) {
          wps = PSTR("WPS");
        }
      }
      Serial.printf(PSTR("  %02d: [CH %02d] [%02X:%02X:%02X:%02X:%02X:%02X] %ddBm %c %c %-11s %3S %s\n"), i, channel, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], rssi, (encryptionType == ENC_TYPE_NONE) ? ' ' : '*', hidden ? 'H' : 'V', phyMode.c_str(), wps, ssid.c_str());
      yield();
    }
  } else {
    Serial.printf(PSTR("WiFi scan error %d"), scanResult);
  }
  }//End scan_Wifi()

bool Find_WPS_Wifi(){
  String ssid = "";
  String MDP = "";
  while(!wifiConnected){
      // put your main code here, to run repeatedly:
      // Called to check if SSID and password has already been stored by previous WPS call.
      // The SSID and password are stored in flash memory and will survive a full power cycle.
      // Calling ("",""), i.e. with blank string parameters, appears to use these stored values.
      // WiFi.begin("","");
      // Long delay required especially soon after power on.
      // delay(1000);
      // Check if WiFi is already connected and if not, begin the WPS process.
      if (WiFi.status() != WL_CONNECTED) {
          Serial.println("\nTesting WPS connection       ...");
          WiFi.beginWPSConfig();
          //WiFi.begin("CGUEBLE","CGUEBLE_PWD");//Pour test du reste de la fonction
          // Another long delay required.
          //delay(2000);
          if (WiFi.status() == WL_CONNECTED) {
              
              //PublicIP = char*(WiFi.localIP());
              ssid = WiFi.SSID();
              MDP = WiFi.psk();
              Serial.println("Wifi WPS found");
              Serial.println("Connected!");
              Serial.println(WiFi.localIP());
              Serial.println(ssid);
              Serial.println(WiFi.macAddress());
              Serial.println(MDP);
              wifiConnected = true;
          }
          else {
            Serial.println("Connection failed!");
            wifiConnected = false;
          }
      }
      else {
        Serial.println("\nConnection already established.");
        wifiConnected = true;
      }
    }
    if(wifiConnected){
      // Call getVcc() and print out voltage (getVcc() returns millivolts so we divide by 1000.00
      BatteryVoltage = ESP.getVcc()/1024.0;
      RSSI = WiFi.RSSI();
      Serial.println("Connection established!");
      MqttPayload = "SSID=" + ssid + " Password=" + MDP;
      return true;
    } 
    else {
      return false;
    } 
  }// Find_WPS_Wifi()
 
void setup_mqtt(){
  setup_LastWill();
  mqttClient.setId(clientId);
  mqttClient.setUsernamePassword(mqttUsername, mqttPassword);
  Serial.println("Connection au serveur MQTT ...");
  while (!mqttClient.connect(mqttServer,mqttPort)) {
    Serial.print("echec");
    Serial.println("nouvel essai dans 1s");
    delay(1000);
  }
  Serial.println("MQTT connecté");
  mqttClient.subscribe(inTopic,mqttQos);
  Serial.print("suscribed to topic:");
  Serial.println(inTopic);
  reconnect();
  mqtt_publish_String(outTopic, "Board Connected");
  }
void setup_LastWill(){
  String willPayload = MqttPayload;
  bool willRetain = true;
  int willQos = 1;
  mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQos);
  mqttClient.print(willPayload);
  mqttClient.endWill();
  }
void mqtt_publish_String(String outTopic, String payloadString){//Fonction pour publier un String sur un topic
  mqttClient.beginMessage(outTopic, payloadString.length(), mqttRetained, mqttQos, dup);
  mqttClient.print(payloadString);
  mqttClient.endMessage();
  Serial.print("MQTT message from mqtt_publish_String: ");
  Serial.println(payloadString);
  }
void reconnect(){
  if (!mqttClient.connected()){
    while (!mqttClient.connect(mqttServer,mqttPort)) {
      Serial.println("Connection au serveur MQTT ...");
      Serial.print("echec");
      Serial.println("nouvel essai dans 60s");
      delay(6000);
    }
    mqttClient.subscribe(inTopic,mqttQos);
    Serial.println("MQTT connecté");
    Serial.print("suscribed to topic:");
    Serial.println(inTopic);
  }

  }
