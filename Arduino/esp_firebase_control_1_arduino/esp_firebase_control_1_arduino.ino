
//-----------------------------------------------------------------------
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
//-----------------------------------------------------------------------
// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing
// info and other helper functions.
#include <addons/RTDBHelper.h>
#include <Pushbutton.h>
#include "ACS712.h"

#include <IRremote.h>
#define IR_SEND_PIN     34
#define DELAY_AFTER_SEND 2000
//-----------------------------------------------------------------------
/* 1. Define the WiFi credentials */
#define WIFI_SSID "network 2"
#define WIFI_PASSWORD "ahmed@@@010"
//-----------------------------------------------------------------------
/* 2. Define the API Key */
#define API_KEY "AIzaSyAcTuv0zBi43Ru9m0Yn47hgFMK9JVb-4no"
//-----------------------------------------------------------------------
/* 3. Define the RTDB URL */
#define DATABASE_URL "https://smart-control-264fa-default-rtdb.firebaseio.com/"
//-----------------------------------------------------------------------
String room_no = "lab1";
//-----------------------------------------------------------------------
// define the GPIO connected with Relays and switches
#define Relay1 2  // D1 //lamp1
#define Relay2 21 // D3 //lamp2
#define Relay3 19 // D3 //fan1
#define Relay4 18 // D3 //fan2
#define Relay5 23 // D3 // AC
#define Relay6 4  // D2 //tv
#define Relay7 15 // D0 // projector
#define Relay8 5  // D3 

#define Switch1 32 // D5
#define Switch2 33 // D6
#define Switch3 25 // SD2
#define Switch4 26 // SD3
#define Switch5 27 // SD4
#define Switch6 14 // SD5
#define Switch7 12 // SD6
#define Switch8 13 // SD7

ACS712  ACS(35, 3.3, 4095, 100);

unsigned long previousMillis = 0;
const long interval = 1500;

Pushbutton btn1(Switch1, true, LOW);
Pushbutton btn2(Switch2, true, LOW);
Pushbutton btn3(Switch3, true, LOW);
Pushbutton btn4(Switch4, true, LOW);
Pushbutton btn5(Switch5, true, LOW);
Pushbutton btn6(Switch6, true, LOW);
Pushbutton btn7(Switch7, true, LOW);
Pushbutton btn8(Switch8, true, LOW);

#define WIFI_LED 2

int stateRelay1 = 0, stateRelay2 = 0, stateRelay3 = 0, stateRelay4 = 0, stateRelay5 = 0, stateRelay6 = 0, stateRelay7 = 0, stateRelay8 = 0;
//-----------------------------------------------------------------------

/* Uncomment only if, you have selected the email authentication from
  firebase authentication settings */
//#define USER_EMAIL "ENTER_USER_EMAIL"
//#define DATABASE_URL "USER_PASSWORD"
//-----------------------------------------------------------------------

/***********************************************************
  0. complete_path = /room1/L1 or /room1/L2 etc.
    this is the complete path to firebase database
  1. stream_path = /room1
    this is the top nodes of firebase database
***********************************************************/
String stream_path = "";
/***********************************************************
  2. event_path = /L1 or /L2 or /L3 or /L4
    this is the data node in firbase database
***********************************************************/
String event_path = "";
/***********************************************************
  3. stream_data - use to store the current command to
    turn ON or OFF the relay
***********************************************************/
String stream_data = "";
/***********************************************************
  5. jsonData - use to store "all the relay states" from
    firebase database jsonData object used only once when
    you reset the nodemcu or esp32 check the following:
    else if(event_path == "/") in the loop() function
***********************************************************/
FirebaseJson jsonData;

/***********************************************************
  it becomes TRUE when data is changed in Firebase
  used in streamCallback function
***********************************************************/
volatile bool dataChanged = false;

/***********************************************************
  this variable is based on the authentication method you
  have selected while making firebase database project.
  authentication method: anonymus user
***********************************************************/
bool signupOK = false;

/***********************************************************
  resetPressed variable is used only once when you
  pressed the reset button. it is used to send test data to
  Firebase database. If we will not test send data then the
  project will not work. used in the loop function.
***********************************************************/
bool resetPressed = true;

/***********************************************************
  when there is some data to upload to theFirebase
  then the value of uploadBucket will TRUE. This
  variable is used in listenSwitches() function
***********************************************************/
bool uploadBucket = false;

// i.e bucketData = "1" or "0"
// i.e bucketPath = "L1" or "L2" etc.
String bucketData = "", bucketPath = "";

//-----------------------------------------------------------------------
// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
/******************************************************************************************
   void reloadRelayStates()
   This function execute
   1. when you press the reset button of the microcontroller. OR
   2. when data is received in json format
 ******************************************************************************************/
void reloadRelayStates()
{
  // jsonData.toString(Serial, true);
  String strKey, strVal;
  //_________________________________________________________
  size_t count = jsonData.iteratorBegin();
  for (size_t i = 0; i < count; i++)
  {
    FirebaseJson::IteratorValue value = jsonData.valueAt(i);
    //-----------------------------------
    strVal = value.value.c_str();
    strVal.replace("\\", "");
    strVal.replace("\"", "");
    strKey = value.key.c_str();
    Serial.println("strKey:" + strKey);
    Serial.println("strVal:" + strVal);
    //-----------------------------------
    if (strKey == "L1")
    {
      stateRelay1 = strVal.toInt();
      digitalWrite(Relay1, stateRelay1);
      // Serial.print("relay1:");Serial.println(stateRelay1);
    }
    else if (strKey == "L2")
    {
      stateRelay2 = strVal.toInt();
      digitalWrite(Relay2, stateRelay2);
      // Serial.print("relay2:");Serial.println(stateRelay2);
    }
    else if (strKey == "L3")
    {
      stateRelay3 = strVal.toInt();
      digitalWrite(Relay3, stateRelay3);
      // Serial.print("relay3:");Serial.println(stateRelay3);
    }
    else if (strKey == "L4")
    {
      stateRelay4 = strVal.toInt();
      digitalWrite(Relay4, stateRelay4);
      // Serial.print("relay4:");Serial.println(stateRelay4);
    }

    else if (strKey == "L5")
    {
      stateRelay5 = strVal.toInt();
      digitalWrite(Relay5, stateRelay5);
      // Serial.print("relay5:");Serial.println(stateRela5);
    }
    else if (strKey == "L6")
    {
      stateRelay6 = strVal.toInt();
      digitalWrite(Relay6, stateRelay6);
        IrSender.sendNEC(0x2207, 0x10, 0);
      // Serial.print("relay6:");Serial.println(stateRelay6);
    }
    else if (strKey == "L7")
    {
      stateRelay7 = strVal.toInt();
      digitalWrite(Relay7, stateRelay7);
      // Serial.print("relay7:");Serial.println(stateRelay7);
    }
    else if (strKey == "L8")
    {
      stateRelay8 = strVal.toInt();
      digitalWrite(Relay8, stateRelay8);
      // Serial.print("relay8:");Serial.println(stateRelay8);
    }
  }
  // required for free the used memory in iteration (node data collection)
  jsonData.iteratorEnd();
  jsonData.clear();
  //_________________________________________________________
}

void listenSwitches()
{
  // String URL = room_no;
  if (btn1.getSingleDebouncedPress())
  {
    stateRelay1 = !stateRelay1;
    digitalWrite(Relay1, stateRelay1);
    uploadBucket = true;
    bucketData = String(stateRelay1);
    bucketPath = "L1";
  }
  else if (btn2.getSingleDebouncedPress())
  {
    stateRelay2 = !stateRelay2;
    digitalWrite(Relay2, stateRelay2);
    uploadBucket = true;
    bucketData = String(stateRelay2);
    bucketPath = "L2";
  }
  else if (btn3.getSingleDebouncedPress())
  {
    stateRelay3 = !stateRelay3;
    digitalWrite(Relay3, stateRelay3);
    uploadBucket = true;
    bucketData = String(stateRelay3);
    bucketPath = "L3";
  }
  else if (btn4.getSingleDebouncedPress())
  {
    stateRelay4 = !stateRelay4;
    digitalWrite(Relay4, stateRelay4);
    uploadBucket = true;
    bucketData = String(stateRelay4);
    bucketPath = "L4";
  }
  else if (btn5.getSingleDebouncedPress())
  {
    stateRelay5 = !stateRelay5;
    digitalWrite(Relay5, stateRelay5);
    uploadBucket = true;
    bucketData = String(stateRelay5);
    bucketPath = "L5";
  }
  else if (btn6.getSingleDebouncedPress())
  {
    stateRelay6 = !stateRelay6;
    digitalWrite(Relay6, stateRelay6);
    uploadBucket = true;
    bucketData = String(stateRelay6);
    bucketPath = "L6";
  }
  else if (btn7.getSingleDebouncedPress())
  {
    stateRelay7 = !stateRelay7;
    digitalWrite(Relay7, stateRelay7);
    uploadBucket = true;
    bucketData = String(stateRelay7);
    bucketPath = "L7";
  }
  else if (btn8.getSingleDebouncedPress())
  {
    stateRelay8 = !stateRelay8;
    digitalWrite(Relay8, stateRelay8);
    uploadBucket = true;
    bucketData = String(stateRelay8);
    bucketPath = "L8";
  }
}

void FirebaseWrite(String URL, int data)
{
  /********************************************************************************************
     Data at a specific node in Firebase RTDB can be read through the following functions:
     get, getInt, getFloat, getDouble, getBool, getString, getJSON, getArray, getBlob, getFile
   ********************************************************************************************/
  if (!Firebase.ready() && !signupOK)
  {
    Serial.println("Write Failed: Firebase not ready OR signup not OK");
    return;
  }

  URL = room_no + "/" + URL;
  Serial.println(URL);
  Serial.println(String(data));
  // Write an Int number on the database path (URL) room1/L1, room1/L2 etc.
  if (Firebase.RTDB.setString(&fbdo, URL, String(data)))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  // FirebaseJson json;
  // json.add(URL, String(data));

  // URL = room_no;
  // Serial.printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&fbdo, URL, &json) ? "ok" : fbdo.errorReason().c_str());
}
/***************************************************************************************/
/*************************** READ SENSOR ***************************************/
/*******************************************************************************/
unsigned int readSensor() {

  unsigned int sval = 0;
  for (int i = 0; i < 10; i++) {
    sval += ACS.mA_AC();
  }
  if ((sval / 10) < 10) {
    return 0;
  }
  else return sval / 10;
}
/*
  /******************************************************************************************
   void streamCallback(FirebaseStream data)
   This function execute automatically
   1. when you press the reset button of the microcontroller.
   2. when any data is changed in the firebase basebase.
   microcontroller.
 ******************************************************************************************/
void streamCallback(FirebaseStream data)
{
  stream_path = data.streamPath().c_str();
  event_path = data.dataPath().c_str();

  if (String(data.dataType().c_str()) == "json")
  {
    jsonData = data.to<FirebaseJson>();
  }
  else
  {
    // intData(), floatData()
    stream_data = data.stringData();
  }
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                stream_path,
                event_path,
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since
  // the stream connected and read once and will not update until
  // stream reconnection takes place. This max value will be zero
  // as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n",
                data.payloadLength(), data.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that
  // used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

/******************************************************************************************
   void streamTimeoutCallback(bool timeout)
 ******************************************************************************************/
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(),
                  stream.errorReason().c_str());
}

/******************************************************************************************
   void setup()
 ******************************************************************************************/
void setup()
{
  Serial.begin(115200);
  //-----------------------------------------------------------------------
  pinMode(Relay1, OUTPUT);
  digitalWrite(Relay1, LOW);
  pinMode(Relay2, OUTPUT);
  digitalWrite(Relay2, LOW);
  pinMode(Relay3, OUTPUT);
  digitalWrite(Relay3, LOW);
  pinMode(Relay4, OUTPUT);
  digitalWrite(Relay4, LOW);
  pinMode(Relay5, OUTPUT);
  digitalWrite(Relay5, LOW);
  pinMode(Relay6, OUTPUT);
  digitalWrite(Relay6, LOW);
  pinMode(Relay7, OUTPUT);
  digitalWrite(Relay7, LOW);
  pinMode(Relay8, OUTPUT);
  digitalWrite(Relay8, LOW);

  pinMode(WIFI_LED, OUTPUT);

  ACS.autoMidPoint();
  IrSender.begin(34, ENABLE_LED_FEEDBACK);

  Serial.print("MidPoint: ");
  Serial.print(ACS.getMidPoint());
  Serial.print(". Noise mV: ");
  Serial.println(ACS.getNoisemV());

  //-----------------------------------------------------------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //-----------------------------------------------------------------------
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  //-----------------------------------------------------------------------
  /*Or Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase signUp ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  //-----------------------------------------------------------------------

  /*Or Assign the user sign in credentials */
  // auth.user.email = USER_EMAIL;
  // auth.user.password = USER_PASSWORD;

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";
  //-----------------------------------------------------------------------
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //-----------------------------------------------------------------------
  // Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif
  //-----------------------------------------------------------------------
  if (!Firebase.RTDB.beginStream(&stream, room_no))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  //-----------------------------------------------------------------------
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  //-----------------------------------------------------------------------

  //  ACS.autoMidPoint();
  //  ACS.getMidPoint();
  //  ACS.getNoisemV();
}

/******************************************************************************************
   void loop()
 ******************************************************************************************/
void loop()
{
  unsigned long currentMillis = millis();
  //--------------------------------------------------------------------------
  if (WiFi.status() != WL_CONNECTED)
  {
    // Serial.println("WiFi Not Connected");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  }
  // else
  // {
  //   // Serial.println("WiFi Connected");
  //   digitalWrite(WIFI_LED, LOW); // Turn on WiFi LED
  // }
  //--------------------------------------------------------------------------
  listenSwitches();
  if (Firebase.ready() && signupOK)
  {
    if (uploadBucket == true)
    {
      uploadBucket = false;
      String URL = room_no + "/" + bucketPath;
      Serial.println(URL);
      Serial.printf("Set String... %s\n\n", Firebase.RTDB.setString(&fbdo, URL, bucketData) ? "ok" : fbdo.errorReason().c_str());
    }
    if (resetPressed == true)
    {
      resetPressed = false;
      Serial.printf("Set Test String... %s\n\n", Firebase.RTDB.setString(&fbdo, "/test", "0") ? "ok" : fbdo.errorReason().c_str());
    }
    bucketPath = "";
    bucketData = "";
  }
  //--------------------------------------------------------------------------
  if (dataChanged)
  {
    dataChanged = false;
    Serial.println("dataChanged");
    // When stream data is available, do anything here...
    //____________________________________________________________________
    // delete \ and " from the data_stream string
    //  \"2\"
    stream_data.replace("\\\"", "");
    //____________________________________________________________________
    if (event_path == "/L1")
    {
      Serial.println("relay1:" + stream_data);
      stateRelay1 = stream_data.toInt();
      digitalWrite(Relay1, stateRelay1);
    }
    //____________________________________________________________________
    else if (event_path == "/L2")
    {
      Serial.println("relay2:" + stream_data);
      stateRelay2 = stream_data.toInt();
      digitalWrite(Relay2, stateRelay2);
    }
    //____________________________________________________________________
    else if (event_path == "/L3")
    {
      Serial.println("relay3:" + stream_data);
      stateRelay3 = stream_data.toInt();
      digitalWrite(Relay3, stateRelay3);
    }
    //____________________________________________________________________
    else if (event_path == "/L4")
    {
      Serial.println("relay4:" + stream_data);
      stateRelay4 = stream_data.toInt();
      digitalWrite(Relay4, stateRelay4);
    }
    else if (event_path == "/L5")
    {
      Serial.println("relay45:" + stream_data);
      stateRelay5 = stream_data.toInt();
      digitalWrite(Relay5, stateRelay5);
    }
    else if (event_path == "/L6")
    {
      Serial.println("relay6:" + stream_data);
      stateRelay6 = stream_data.toInt();
      digitalWrite(Relay6, stateRelay6);
  IrSender.sendNEC(0x2207, 0x10, 0);
    
    }
    else if (event_path == "/L7")
    {
      Serial.println("relay7:" + stream_data);
      stateRelay7 = stream_data.toInt();
      digitalWrite(Relay7, stateRelay7);
    }
    else if (event_path == "/L8")
    {
      Serial.println("relay8:" + stream_data);
      stateRelay4 = stream_data.toInt();
      digitalWrite(Relay8, stateRelay8);
    }
    //____________________________________________________________________
    else if (event_path == "/")
    {
      // this if statement executes only once if you reset the nodemcu or esp32.
      // The datachange event automatically occurs when you reset microcontroller
      // jsonData object is used in the below called function
      reloadRelayStates();
    }
    //____________________________________________________________________
    stream_data = "";
  }
  //--------------------------------------------------------------------------
  //  FirebaseWrite("mA", readSensor());
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    FirebaseWrite("CONNECTED", true);
    FirebaseWrite("mA", readSensor()<320?0:readSensor());
//    FirebaseWrite("mA", ACS.mA_AC());
    //Serial.println(ACS.mA_AC());
  }

}
