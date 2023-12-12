const int buttonPin = 5;//button pin = hall input
int buttonState;
int wifiled = 27;
int bootled = 26;
int notiled = 25;
int sourcled = 33;
bool wifiCon = false;
bool conState = false;
bool hotState = false; 
bool lock_state = false;
bool n = true;
int sor = 32;
bool sorState = false;
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
const String key = "key=AAAAdYvgZJE:APA91bHZFgCSvN_8n_QSr12ytwPkVutGt9YP6BQ_OGP5g4wSK6LpUJfz1rAvgBC4CgAB987LfBgZXImZa5AdkyG5r_0KwQOSv1UY25yXwZ5tgPaH4Hajl83h8pMS-eet7FUPWu8FW1XF";
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"



// Insert Firebase project API Key
#define API_KEY "AIzaSyArsNWk5kY2JcF11qtwk5oeLHi3kKl76N8"
#define DATABASE_URL "https://rodent-box-default-rtdb.firebaseio.com/" 

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "test@gmail.com"
#define USER_PASSWORD "12345678"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Initialize WiFi
void initWiFi() {
WiFiManager wm;
bool res;
res = wm.autoConnect("Rodent Box");

// anonymous ap
if(!res) {
    Serial.println("Failed to connect");
        // ESP.restart();
//        wifiled = LOW;
    digitalWrite(wifiled,HIGH);
   conState = false;
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
//        wifiled = HIGH;
    digitalWrite(wifiled,LOW);
    }
}

void setup(){
  bootLed();
  Serial.begin(115200);  
  //
  pinMode(wifiled,OUTPUT);
  pinMode(bootled,OUTPUT);
  pinMode(notiled,OUTPUT);
  pinMode(sourcled,OUTPUT);
  pinMode(sor,INPUT_PULLUP);
  // Initialize WiFi
  initWiFi();
  
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;
  config.database_url = DATABASE_URL;
  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  if(uid != "")
  {
    conState = true;
    wifiCon = false;
    }
  Serial.print("User UID: ");
  Serial.print(uid);
   pinMode(buttonPin, INPUT_PULLUP);
   //pinMode(led, OUTPUT);
}
void loop(){
//power stat
charge();
wifLed();
if (Firebase.RTDB.getBool(&fbdo, "User/"+uid+"/isPoweredOn/")) {
        bool  intValue = fbdo.boolData();
      if (!fbdo.boolData()) {
        Serial.println("Pushed Powered on");
        writeData("isPoweredOn",true);
      }   
}  
buttonState = digitalRead(buttonPin);
if(lock_state)
{
  if (buttonState == HIGH)  {
  lock_state=!lock_state;
  //digitalWrite(LED_BUILTIN, LOW);   
  Serial.println("UnLocked");
  writeData("isLocked",false);
  
    }
  }
  if(!lock_state)
{
  if (buttonState == LOW)
  {
  lock_state=!lock_state;
  //digitalWrite(LED_BUILTIN, HIGH);   
  Serial.println("locked");
  writeData("isLocked",true);
  sendNotification();
  
    }
  }

if(!sorState)
{
  if (digitalRead(sor) == HIGH)  {
  sorState=!sorState;
  //digitalWrite(LED_BUILTIN, LOW);   
  Serial.println("battery");
  writeData("isCharging",false);
  
    }
  }
  if(sorState)
{
  if (digitalRead(sor) == LOW)  {
  sorState=!sorState;
  //digitalWrite(LED_BUILTIN, HIGH);   
  Serial.println("charger");
  writeData("isCharging",true);
 // sendNotification();
  
    }
  }

  
}
bool writeData(String path,bool stat)
{
  if (Firebase.RTDB.setInt(&fbdo, "User/"+uid+"/"+path+"/", stat)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      return true;
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      return false;
    }
    Serial.println("done."); 
  }
    int sendNotification()
{
  notiLed();
  HTTPClient http;        
  http.begin("https://fcm.googleapis.com/fcm/send");
  http.addHeader("Authorization",key);
  http.addHeader("Content-Type","application/json");
  Serial.print("Sending notification ! ==> ");  
  //String data = String("{\"to\":\"")+device+"\",\"time_to_live\":60,\"priority\":\"high\",\"data\":{\"text\":{\"title\":\""+title+"\",\"message\":\""+message+"\",\"clipboard\":false}}}";
   //String data = String("{\"registration_ids\":[\"czblOXjKSBuQFzIgbT9rm8:APA91bF2r3oZelzYHSokHqom4jXiAYQz6gP43wP0YsXpHzMHJSWgtXQ1abo_M3alnTqlodv_LU-vPRHvUqurJqhncaPHbixlzPLeNHiq_Oj4p5SdCf8Ha4VKaTbAZyn2U1FJUBlE2mUk\"],\"notification\":{\"title\":\"IoT Rodent Box\",\"body\":\"Caught a Rodent\"}}");
  String data = String("{\"to\":\"/topics/pushNotifications\",\"notification\":{\"title\":\"Rodent Box\",\"body\":\"Caught a Rodent!\"}}");
  int httpCode = http.POST(data);
  if(httpCode == HTTP_CODE_OK){
    Serial.println("OK");
  }else{
    Serial.println("Failed");
  }
  http.end();
  return httpCode;
  
}
int notiLed()
{

  int i=5;
  while(i>0){
  digitalWrite(notiled,HIGH);
  delay(50);
  digitalWrite(notiled,LOW);
  delay(50);
  i=i-1;
  }
  }  
int wifLed()
{
  if(conState){
  digitalWrite(wifiled,HIGH);
      delay(60);
    digitalWrite(wifiled,LOW);
  }
}
int bootLed()
{
  digitalWrite(bootled,HIGH);
  }
int charge()
{
  if(!sorState)
  {
    digitalWrite(sourcled,HIGH);
    
    }
    else{
      digitalWrite(sourcled,HIGH);
      delay(300);
      digitalWrite(sourcled,LOW);
      delay(300);
      
      }
  }


  
