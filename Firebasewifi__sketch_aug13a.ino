/*
   Note: The latest JSON library might not work with the code. 
   So you may need to downgrade the library to version v5.13.5
   
   Created by TAUSEEF AHMED
   
   YouTube: https://www.youtube.com/channel/UCOXYfOHgu-C-UfGyDcu5sYw/

   Github: https://github.com/ahmadlogs/
   
*/

//-----------------------------------------------------------------------------------
//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include <FirebaseESP8266.h>  //https://github.com/mobizt/Firebase-ESP8266
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h> //https://github.com/mikalhart/TinyGPSPlus
#include <Dictionary.h>

//Install ArduinoJson Library
//Note: The latest JSON library might not work with the code. 
//So you may need to downgrade the library to version v5.13.5
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
#define FIREBASE_HOST "https://esp8266-gps-eb6cd-default-rtdb.firebaseio.com" 
#define FIREBASE_AUTH "2hbZ4UWHQ3KXnUAiKRjP0C84fmMIYc5eJdJw95DJ"
#define WIFI_SSID "UIU-STUDENT"
#define WIFI_PASSWORD "12345678"
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
//Define FirebaseESP8266 data object
FirebaseData firebaseData;

FirebaseJson json;
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//GPS Module RX pin to NodeMCU D1
//GPS Module TX pin to NodeMCU D2
const int RXPin = 4, TXPin = 5;
SoftwareSerial neo6m(RXPin, TXPin);
TinyGPSPlus gps;
//-----------------------------------------------------------------------------------


  
// passenger format:  passenger[tag] = "startLAT;startLON"
//          example:  passenger["12 34 56 78"] = "51.508131;-0.128002"
Dictionary &passenger = *(new Dictionary(24));

// stores the distance
unsigned long distanceKm;

// stores the price
double price;
 
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
static void smartdelay_gps(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (neo6m.available())
      gps.encode(neo6m.read());
  } while (millis() - start < ms);
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void print_ok()
{
    Serial.println("------------------------------------");
    Serial.println("OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void print_fail()
{
    Serial.println("------------------------------------");
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void firebaseReconnect()
{
  Serial.println("Trying to reconnect");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void setup()
{

  Serial.begin(115200);

  neo6m.begin(9600);
  
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

  Serial.println("Connecting Firebase.....");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase OK.");
    
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void loop() {
  
  smartdelay_gps(1000);

  if(gps.location.isValid()) 
  {
    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    
    Serial.print(F("- latitude: "));
    Serial.println(latitude);

    Serial.print(F("- longitude: "));
    Serial.println(longitude);
  
    // we assume that RFID scanned the card and got the tag "12 34 56 78"
    char tag[] = "12 34 56 78";
    
    // test passenger
    //passenger(tag, "51.508131;-0.128002");
  
    // this is true if the tag exists in the dictionary (which means this passenger scanned his tag before, and is now getting off the bus
    if (passenger(tag) == 1){
      // ToDo: find distance
      String startLocation;
      startLocation = passenger[tag];
      int index = startLocation.indexOf(';');
      String LAT = startLocation.substring(0,index);
      String LON = startLocation.substring(index+1,startLocation.length());
      double startLAT = LAT.toDouble();
      double startLON = LON.toDouble();
      distanceKm = TinyGPSPlus::distanceBetween(latitude, longitude, startLAT, startLON) / 1000;
      Serial.print(F("Distance: "));
      Serial.println(distanceKm);
      price = distanceKm * 2.20;
      Serial.print(F("Price: "));
      Serial.println(price);
      delay(1000);
    }
    // if the tag does not exist, it means this passenger is just now getting on the bus (beginning his journey), so let's add him to the passenger dictionary
    else{
      // insert GPS Latitude here
      String startLAT = String(latitude);
      // insert GPS Longitude here
      String startLON = String(longitude);
      // concat LAT and LON with a semi-colon(;) in the middle
      String startLocation;
      startLocation.concat(startLAT);
      startLocation.concat(";");
      startLocation.concat(startLON);
      // we already have the tag from the RFID scan earlier, so let's add the tag and the start location to the dictionary here
      passenger(tag, startLocation);
    }
    //-------------------------------------------------------------
    //Send to Serial Monitor for Debugging
    //Serial.print("LAT:  ");
    //Serial.println(latitude);  // float to x decimal places
    //Serial.print("LONG: ");
    //Serial.println(longitude);
    //-------------------------------------------------------------
    
    //-------------------------------------------------------------
    if(Firebase.setFloat(firebaseData, "/GPS/f_latitude", latitude))
      {print_ok();}
    else
      {print_fail();}
    //-------------------------------------------------------------
    if(Firebase.setFloat(firebaseData, "/GPS/f_longitude", longitude))
      {print_ok();}
    else
      {print_fail();}
    //-------------------------------------------------------------
    if(Firebase.setFloat(firebaseData, "/GPS/distanceKm", distanceKm))
      {}
    else
      {}
    //-------------------------------------------------------------
    if(Firebase.setFloat(firebaseData, "/GPS/price", price))
      {}
    else
      {}
    //-------------------------------------------------------------
  }
  else
  {
    Serial.println("No valid GPS data found.");
  }
  
  delay(5000);
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
