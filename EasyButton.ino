/*  Creates a VM on Morpheus with a button press.
 *  Change wireless variables
 *  Change username/pass for Morpheus
 *  Created By: Chris Bunge/cbunge@morpheusdata.com 04/12/2020
 */

#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "WiFiClientSecure.h"

//WiFi Variables
  const char* ssid = "YOURSSIDHERE"; //SSID Name
  const char* password = "YOURPASSWORDHERE"; //SSID Password

//Morpheus Variables
  const int httpsPort = 443;
  const char* host = "YOURMORPHEUSURLHERE"; //Morpheus URL ex: morpheus.domain.com
  const char* morpheusAuth = "/oauth/token?grant_type=password&scope=write&client_id=morph-customer";
  const char* morpheusInstance = "/api/instances";
  #define morpheusUser "YOURUSERNAMEHERE" // Morpheus User with Access
  #define morpheusPW "YOURUSERPASSWORDHERE" // Morpheus User Password
  #define DEBUG false // Debug True False

//Input Variables
  #define BLUE 4
  #define GREEN 5
  #define RED 16
  const int buttonPin = 15; 
  int buttonState = 0; 

//Define Color Vars
  int redValue;
  int greenValue;
  int blueValue;

//Functions
  //Submit Payload
  String postRequest(const char* server, String head, String data) {
  #ifdef DEBUG
    Serial.print("Function: "); Serial.println("postRequest()");
  #endif
  
    String result = "";
  
    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    client.setInsecure();
    Serial.print("Connecting To: "); Serial.println(server);
  
    if (!client.connect(server, httpsPort)) {
      Serial.println("Connection Failed");
      return result;
    }
  
    if(client.verifyCertChain(server)) {
  
      #ifdef DEBUG
          Serial.print("Post: "); Serial.println(head + data);
      #endif
    
      client.print(head + data);
    
      #ifdef DEBUG
          Serial.println("Request Sent");
          Serial.println("Receiving Response:");
      #endif
    
      while (client.connected()) {
        if(client.find("HTTP/1.1 ")) {
          String status_code = client.readStringUntil('\r');
          Serial.print("Status code: "); Serial.println(status_code);
          if(status_code != "200 ") {
            Serial.println("An Error Has Occurred!");
            client.stop();
            return result;
          }
        }
        if(client.find("access_token\":\"")) {
          Serial.println("Successfully Grabbed Bearer Token...");
          String access_token = client.readStringUntil('\"');
          result += access_token;
        
        #ifdef DEBUG
            Serial.print("Access Token: "); Serial.println(access_token);
        #endif  
        }
      }
      return result;
    }
  }
  
void setup()
{
  //Pin Setup
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  pinMode(buttonPin, INPUT);
  
  redValue = 255; // choose a value between 1 and 255 to change the color.
  greenValue = 0;
  blueValue = 0;

  Serial.begin(9600);
  // Connect to WiFi
  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  
  // WiFi Connected, LED BLUE
  Serial.println("");
  Serial.println(String("Successfully connected: ") + ssid);
  redValue = 0;
  blueValue = 255;
  analogWrite(RED, redValue);
  analogWrite(BLUE, blueValue);
  
  //Print IP
  Serial.println("");
  Serial.println(WiFi.localIP());
}

void loop() {
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {
    WiFiClientSecure client;
    client.setInsecure();

    //Button Press
    Serial.println("");
    Serial.println("VM Build Starting");
    greenValue = 255;
    blueValue = 0;
    analogWrite(GREEN, greenValue);
    analogWrite(BLUE, blueValue);
    
    //Step 1: OAUTH Request
  
      //Create Body
      String authData = "";
      authData += F("username=");
      authData += morpheusUser;
      authData += F("&password=");
      authData += morpheusPW;
  
      //Create Header
      String authHeader = "";
      authHeader += (String("POST ") + morpheusAuth + " HTTP/1.1\r\n");
      authHeader += (String("Host: ") + host + "\r\n");
      authHeader += (F("Connection: close\r\n"));
      authHeader += (F("Accept: */*\r\n"));
      authHeader += (F("Content-Type: application/x-www-form-urlencoded\r\n"));
      authHeader += ("Content-Length: ");
      authHeader += (authData.length());
      authHeader += ("\r\n\r\n");
  
      //Submit OAUTH Payload
      String token = postRequest(host, authHeader, authData);
  
    //Step 2: VM Build Request
  
      //Create Body
      String postData = "";
      postData += "{\r\n\"zoneId\":1,\r\n\"instance\":{\r\n\"name\":\"EasyButton-${sequence}\",\r\n\"cloud\":\"vCenter\",\r\n\"site\":{\r\n\"id\":7\r\n},\r\n\"type\":\"cent\",\r\n\"instanceType\":{\r\n\"code\":\"cent\"\r\n},\r\n\"description\":\"CreatedwithEasyButtonAPI!\",\r\n\"instanceContext\":\"qa\",\r\n\"layout\":{\r\n\"id\":3150\r\n},\r\n\"plan\":{\r\n\"id\":889,\r\n\"code\":\"vm-512\",\r\n\"name\":\"1CPU, 512MB Memory\"\r\n},\r\n\"networkDomain\":{\r\n\"id\":null\r\n}\r\n},\r\n\"config\":{\r\n\"resourcePoolId\":12,\r\n\"noAgent\":\"off\",\r\n\"smbiosAssetTag\":null,\r\n\"nestedVirtualization\":\"off\",\r\n\"hostId\":\"\",\r\n\"vmwareFolderId\":null,\r\n\"vmwareCustomSpec\":null,\r\n\"createUser\":true\r\n},\r\n\"plan\":{\r\n\"id\":889,\r\n\"code\":\"vm-512\",\r\n\"name\":\"1CPU,512MBMemory\"\r\n},\r\n\"volumes\":[\r\n{\r\n\"id\":-1,\r\n\"rootVolume\":true,\r\n\"name\":\"root\",\r\n\"size\":10,\r\n\"sizeId\":null,\r\n\"storageType\":1,\r\n\"datastoreId\":\"auto\",\r\n\"hasDatastore\":true\r\n}\r\n],\r\n\"networkInterfaces\":[\r\n{\r\n\"network\":{\r\n\"id\":\"network-100\"\r\n}\r\n}\r\n]\r\n}";
  
      //Create Header
      String postHeader = "";
      postHeader += (String("POST ") + morpheusInstance + " HTTP/1.1\r\n");
      postHeader += (String("Host: ") + host + "\r\n");
      postHeader += (F("Connection: close\r\n"));
      postHeader += (F("Accept: */*\r\n"));
      postHeader += (F("Authorization: Bearer "));
      postHeader += (token);
      postHeader += ("\r\n");
      postHeader += (F("Content-Type: application/json\r\n"));
      postHeader += ("Content-Length: ");
      postHeader += (postData.length());
      postHeader += ("\r\n\r\n");
  
      //Submit VM Payload
      Serial.println("Submitting VM Building JSON");
      postRequest(host, postHeader, postData);

    //Cleanup
      client.stop();
      Serial.println("");
      Serial.println("Build Request Process Complete!");
      greenValue = 0;
      blueValue = 255;
      analogWrite(GREEN, greenValue);
      analogWrite(BLUE, blueValue);      
  }
}