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

void setup() {
  Serial.begin(115200);
  // WPS works in STA (Station mode) only.
  WiFi.mode(WIFI_STA);
  //delay(100);
}
void(* resetFunc) (void) = 0;


byte sendEmail(String FcMailFrom, String FcMailTo, String FcMailContent)
{
  Serial.print("server_email=");
  Serial.println(server_email);
  Serial.print("FcMailFrom=");
  Serial.println(FcMailFrom);
  Serial.print("FcMailTo=");
  Serial.println(FcMailTo);
  if (client.connect(server_email, 465)) {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("mail server connected");
    }
  } else {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("mail server connection failed");
    }
    return 0;
  }

  if (!eRcv()) return 0;

  // change to your public ip
  client.println("helo 86.238.250.218");

  if (!eRcv()) return 0;
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending From");
  }

  // change to your email address (sender)
  client.print("MAIL From:");
  client.println(FcMailTo);// contient l'adresse email de l'expediteur "<toto@titi.fr>"

  if (!eRcv()) return 0;

  // change to recipient address
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending To");
  }
  client.print("RCPT To: ");
  client.println(FcMailTo);// contient l'adresse email du destinataire "<toto@titi.fr>"
  if (!eRcv()) return 0;

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending DATA");
  }

  client.println("DATA");
  if (!eRcv()) return 0;


  //*****************Creation du Mail***********************
  client.print("To: You "); // Destinataire
  client.println(FcMailTo); // Destinataire
  client.print("From: Me "); // My address
  client.println(FcMailFrom); // My address
  client.println("Subject: Arduino Report from Maison Paris\r\n"); //sujet du mail
  //Corps du mail
  client.print(FcMailContent); // message specifique de l'application appelante

  
  client.print("http:///");
  client.println(WiFi.localIP());

  client.print("SW version: "); // Date de compilation
  client.println("   ;   WPS spy jan- 2025"); // chemin

  //fin du mail
  client.println(".");
  if (!eRcv()) return 0; //test si mail correctement envoye

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending email");
  }

  client.println("QUIT"); //Deconnection du server mail
  if (!eRcv()) return 0;
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending QUIT");
  }
  client.stop();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("disconnected");
  }
  return 1;

}

byte eRcv(){
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("\r\nTimeout");
      }
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.write(thisByte);
    }
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println("QUIT");

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("\r\nTimeout");
      }
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.write(thisByte);
    }
  }
  client.stop();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("disconnected");
  }
}

void loop() {
  while(!wifiConnected){
    // put your main code here, to run repeatedly:
    // Called to check if SSID and password has already been stored by previous WPS call.
    // The SSID and password are stored in flash memory and will survive a full power cycle.
    // Calling ("",""), i.e. with blank string parameters, appears to use these stored values.
    //WiFi.begin("","");
    // Long delay required especially soon after power on.
    delay(1000);
    // Check if WiFi is already connected and if not, begin the WPS process.
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nAttempting connection ...");
        WiFi.beginWPSConfig();
        // Another long delay required.
        delay(3000);
        if (WiFi.status() == WL_CONNECTED) {
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
  Serial.println("Connection established! => Send Mail");
  MailContent = "FcMailContent a completer avec les codes";
  sendEmail(MailFrom, MailTo, MailContent);
  delay(1000);
  //resetFunc();
  setup();
}
