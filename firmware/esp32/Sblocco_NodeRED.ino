#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include "protocol.h"
//#include "wifi_login.h"
#include "device_login.h"
#include "server_url.h"
#include <WiFiManager.h>

#define NR_TIMEOUT  30
#define WF_TIMEOUT  1

#define RXD2 16
#define TXD2 17

#define COM_BAUD 115200

HardwareSerial comSerial(1);

// Variabili globali
bool treatmentRunning = true;

uint8_t RXData;
int incomingByte;

uint8_t prev_wfConn = WL_NO_SHIELD;
uint8_t wfConn = WL_NO_SHIELD;

uint8_t prev_nrConn = 0;
uint8_t nrConn = 0;

unsigned long time1,time2;



int getRemainingTime(void);
int sendConsumedTime(uint32_t timeConsumed, const char* treatment);


bool isEnabled = false;
int timeRem = 0;

int sec_counter=0;

void setup() {
  
  Serial.begin(115200);
  comSerial.begin(COM_BAUD, SERIAL_8N1, RXD2, TXD2);
  comSerial.begin(115200);
  Serial.println("start");

  time1=millis();
  time2=millis();
  
  WiFi.begin();
 // delay (1000);
//  prev_wConn = WiFi.status();

  // Connessione al Wi-Fi
//  Serial.print("Connessione a Wi-Fi");
/*
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
//    Serial.print(".");
  }
  */
//  Serial.println("\nConnesso al Wi-Fi!");

  /*
  if (getWifiConn()==0){
    Serial.println ("CONNESSO AL WIFI");
    if(checkNodeRedConnection()){
      Serial.println ("SERVER ONLINE");
      Serial.print("TEMPO RIMANENTE: ");
      int j = getRemainingTime();
      Serial.println(j);
      if(isEnabled){
        Serial.println("UTENTE ABILITATO");
      }
      else{
        Serial.println("UTENTE NON ABILITATO");
      }
    }
    else{
      Serial.println ("SERVER OFFLINE");      
    }
  }
  else{
    Serial.println ("NON CONNESSO AL WIFI");
  }

*/
}

void loop() {




  time1=millis();
  if(time1-time2>= 1000){
    sec_counter++;
//    Serial.print(".");
    time2=time1;
    wfConn = WiFi.status();
    if(wfConn != WL_CONNECTED){
      sec_counter=0;
    }
    if ((wfConn != prev_wfConn)){//&&((wfConn == WL_CONNECTED)||(prev_wfConn == WL_CONNECTED))){
      prev_wfConn=wfConn;

      if (wfConn != WL_CONNECTED){
        Serial.println("connessione livello 0");
        PCmd_refreshCONN(0);
      }
      else{
        sec_counter = NR_TIMEOUT;
      }
    }
  }
  if (sec_counter >= NR_TIMEOUT){
    sec_counter=0;
//    Serial.print("+");

    HTTPClient http;
    http.begin(serverURL_getConnection);
    
    nrConn = http.GET();
    if(nrConn != prev_nrConn){
      prev_nrConn = nrConn;
      if (nrConn == 200) {
        Serial.println("connessione livello 2");
        PCmd_refreshCONN(2);
      } else {
        Serial.println("connessione livello 1");
        PCmd_refreshCONN(1);
      }
    }

    http.end();
  
  }

  
  
  
  if (comSerial.available() > 0) {
    incomingByte = comSerial.read();
    RXData=(uint8_t)incomingByte;
    checkComData();
  }
}

int getRemainingTime() {
  // Controlla se siamo connessi al Wi-Fi

  HTTPClient http;
  http.begin(serverURL_getRemaining);                   // Inizializza la connessione all'endpoint
  http.addHeader("Content-Type", "application/json"); // Aggiunge l'header per JSON

  // Costruiamo il body JSON con le credenziali
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["username"] = username;
  jsonDoc["password"] = devicePassword;

  String requestBody;
  serializeJson(jsonDoc, requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();

    StaticJsonDocument<200> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);
    if (!error) {
      const char* status = responseDoc["status"];
      if (status && String(status) == "success") {
        timeRem = responseDoc["time_remaining"];
        Serial.print("TEMPO RIMANENTE: ");
        Serial.println(timeRem);
        http.end();
        isEnabled = responseDoc["enabled"] | false; 
        Serial.println("UTENTE ABILITATO");
        return 0;
        
      } else {
        Serial.println("UTENTE NON ABILITATO");
      }
    } else {
       Serial.println("Errore di parsing JSON nella risposta.");
    }
  } else {
      Serial.println("system error");
  }

  http.end();
  return -1;
}

int getWifiConn(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println ("NON CONNESSO AL WIFI");
//    Serial.println("WiFi not connected!!!");
    return -1;
  }
  Serial.println ("CONNESSO AL WIFI");
//  Serial.println("WiFi connected!!!");
  return 0;
}


int sendConsumedTime(uint32_t timeConsumed,  const char* treatment) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Inizializza la connessione all'endpoint Node-RED
    http.begin(serverURL_setConsumedTime);
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");
    
    // Costruisci il payload JSON
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["username"] = username;
    jsonDoc["password"] = devicePassword;
    jsonDoc["time_consumed"] = timeConsumed;
    jsonDoc["treatment"] = treatment;
    
    String jsonData;
    serializeJson(jsonDoc, jsonData);
    
    // Invia la richiesta POST
    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
      String response = http.getString();
      if(httpResponseCode==200){
        //send ackconn
        Serial.println("Sending ACK");
        http.end();
        return 0;

      }
//      Serial.println("Response: " + response);
    } else {
      http.end();
      return -1;
//      Serial.print("Error on sending POST: ");
//      Serial.println(http.errorToString(httpResponseCode));
    }
    
  } else {
//    Serial.println("WiFi not connected");
  }
}

int checkNodeRedConnection() {
  // Verifica prima di tutto la connessione WiFi
  if (WiFi.status() != WL_CONNECTED) {
  //  Serial.println("WiFi non connesso.");
    return -1;  // Errore WiFi
  }

  HTTPClient http;
  http.begin(serverURL_getConnection);
  
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    // Leggiamo la risposta solo se vogliamo ispezionarla
 //   String response = http.getString();
//    Serial.print("Risposta dal server: ");
//    Serial.println(response);

    if (httpResponseCode == 200) {
      Serial.println ("SERVER ONLINE");
//      Serial.println("Connessione OK a Node-RED!");
      http.end();
      return 1; // Connessione riuscita
    }
  } else {
    Serial.println ("SERVER OFFLINE");
//    Serial.print("Errore di connessione HTTP: ");
//    Serial.println(httpResponseCode);
  }

  http.end();
  return 0;  // Errore generico
}

  WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params ) 
void setWiFi(){

    // test custom html(radio)
  const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input
  
  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

    std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  wm.setConnectTimeout(60); // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(10); // auto close configportal after n seconds
    bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }

  // check for button press

        Serial.println("Erasing Config, restarting");
//        wm.resetSettings();
//        ESP.restart();
      
      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);
      
      if (!wm.startConfigPortal("OnDemandAP","password")) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        // ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
      }
}


String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}



