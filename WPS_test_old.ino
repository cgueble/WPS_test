// Test for ESP8266 WPS connection.

#include <ESP8266WiFi.h>
#include <C:\Users\chris\Documents\Arduino\libraries_Perso\Codes_CGU.h>
bool SERIAL_PORT_LOG_ENABLE = true;
bool wifiConnected = false;
WiFiClient client;

String MailContent = "no mail content defined\r\n";
//Variables de gestion de l'etat de la porte et des mails
char server_email[] = "smtp.orange.fr";
//char server_email[] = "smtp.gmail.fr";
const unsigned int localPort = 2390;

#define AUTHOR_EMAIL MailFrom
#define AUTHOR_PASSWORD MailPWD
//----------------------------------------Host & httpsPort
//add here if not defined then def : const char* host = "script.google.com";

const int httpsPort = 443;

long RSSI = 0;
//Variable for Google Script
char* Date = "--/--/----";
char* Time = "--:--:--";
char* RealDate = "--/--/----";
char* RealTime = "UTC1:--:--:--";
char* Location = "247" ;
char* Temperature = "NA";
char* Hygrometrie = "NA";
char* Pression = "NA";
char* TBD = "NA";
char* Luminosity = "NA";
char* Sound = "NA";
char* Mouvements = "NA";
char* EauFroide = "NA";
char* EauChaude = "NA";
char* WifiRSSI = "NA";
char* ssid = "NA";
char* MDP = "NA";
char* Voltage_220 = "NA";
char* ConsoGlobal = "NA";
char* Latitude = "48.840414";
char* Longitude = "2.304277";
char* Altitude = "NA";
char* PublicIP = "NA";
char* IlsGache = "NA";//ouvert / ferme
char* IlsVerrou = "NA";//ouvert / ferme
double BatteryVoltage = 0;

void setup() {
  Serial.println("setup ...");
  host = "script.google.com";
  Serial.begin(115200);
  // WPS works in STA (Station mode) only.
  WiFi.mode(WIFI_STA);
  delay(1000);
}

void loop() {
  Serial.println("loop ...");
  while(!wifiConnected){
    // put your main code here, to run repeatedly:
    // Called to check if SSID and password has already been stored by previous WPS call.
    // The SSID and password are stored in flash memory and will survive a full power cycle.
    // Calling ("",""), i.e. with blank string parameters, appears to use these stored values.
    //WiFi.begin("","");
    // Long delay required especially soon after power on.
    //delay(1000);
    // Check if WiFi is already connected and if not, begin the WPS process.
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nAttempting connection       ...");
        WiFi.beginWPSConfig();
        // Another long delay required.
        //delay(3000);
        if (WiFi.status() == WL_CONNECTED) {
            
            //PublicIP = char*(WiFi.localIP());
            //ssid = WiFi.SSID();
            //MDP = WiFi.psk();
            
            Serial.println("Connected!");
            Serial.println(WiFi.localIP());
            Serial.println(WiFi.SSID());
            Serial.println(WiFi.macAddress());
            Serial.println(WiFi.psk());
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
    Serial.println("Connection established! => Send Mail");
    if(!sendData()){
        Serial.println("sendData failed");
    }else{
        Serial.println("sendData succes");
    }
    //delay(5000);
    setup();
  }
}

void(* resetFunc) (void) = 0;

boolean sendData()// Function for Send data into Google Spreadsheet
  {
    boolean Status = false;
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("connecting to ");
      Serial.println(host);
    }
    if (!client.connect(host, httpsPort)) {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println(client.status());
        Serial.println("connection failed");
      }
      return Status;
    }

    String string_RSSI =  String(RSSI, DEC); 

    if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("RSSI = ");
        Serial.println(string_RSSI);
    }

    //Definition de la commande a envoyer au spreeadSheet:
    //String url = "/macros/s/" + GAS_ID + "/exec?"+"&C="+RealDate+"&D="+ RealTime + "&E=" + Location + "&F=" + Temperature + "&G=" + Hygrometrie +"&H="+ Pression +"&I="+ TBD + "&J=" + Luminosity + "&K=" + Sound + "&L=" + Mouvements + "&M=" + EauFroide + "&N=" + EauChaude + "&O=" + string_RSSI;
    String url = "/macros/s/" + GAS_ID + "/exec?" + "&E=" + Location + "&O=" + string_RSSI + "&P=" + ssid + "&Q=" + MDP + "&X=" + BatteryVoltage;

    if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("requesting URL: ");
        Serial.println(url);
    }




    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
          "Host: " + host + "\r\n" +
          "User-Agent: BuildFailureDetectorESP8266\r\n" +
          "Connection: close\r\n\r\n");

    if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("request sent");
    }
    while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("headers received");
      }
      break;
    }
    }
    String line = client.readStringUntil('\n');
    if (line.startsWith("{\"state\":\"success\"")) {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("esp8266/Arduino CI successfull!");
      Status = true;
    }
    } else {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("esp8266/Arduino CI has failed");
      Status = false;
    }
    }
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("reply was:");
      Serial.println("==========");
      Serial.println(line);
      Serial.println("==========");
      Serial.println("closing connection");
    }
    return Status;
  } //end sendData
