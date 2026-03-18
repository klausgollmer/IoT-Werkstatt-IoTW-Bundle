/*--------------------------------------------------
  Wrapper EDGE Impulse
  MIT
  Martin Seidinger, Nicolas Kiebel, Robin Barton, UCB 
   --------------------------------------------------*/
#ifndef EI_h
#define EI_h
// the #include statment and code go here...

#if defined(ESP8266)
  #include <ESP8266WiFi.h> 
  #include <ESP8266HTTPClient.h> 
#elif defined(ESP32) 
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif
extern int IOTW_debug_level; // Print debug information

// Maximum Datapoints in storage
//#define IOTW_IOTW_EI_MAXPOINTS 50
//#define EI_MAXSENSOR 4

#define AI_CALIBRATION 1
#define AI_VALIDATION 2
#define CONST_e 2.71828
#define AI_MAXMODEL 4
#define AI_MAXMODELPARA 4
#define AI_MODEL_AUTO -1
#define AI_MODEL_LIN 0
#define AI_MODEL_QUAD 1
#define AI_MODEL_EXP 2
#define AI_MODEL_CUB 3


// EDGE-IMPULSE Library by Martin Seidinger, Nicolas Kiebel, Robin Barton, UCB 
//-------------------------EdgeImpulse  ------ HTTP-POST
//void sendEdgeImpulse(String server, String apiKey, String fileName, String deviceTyp, int numberOfDataPoints,
//int numberOfSensors, String dataRate, String* nameOfSensor, String* unitOfSensor, float sensorValues[IOTW_EI_MAXPOINTS][EI_MAXSENSOR], String apiToLoadDataTo) { // Funktion von edgeImpulse Block2

void sendEdgeImpulse(String server, String apiKey, String fileName, String deviceTyp, int numberOfDataPoints,
int numberOfSensors, String dataRate, String* nameOfSensor, String* unitOfSensor, float* sensorValues, String apiToLoadDataTo) { // Funktion von edgeImpulse Block2

  WiFiClient wifiClient;  // WiFiClient-Objekt erstellen



  //Create Json
  //Start of JSON file
  String jsonValue = "{\n";
  
  //Information about the signature format.
  jsonValue += "\"protected\": {\n";
  
  //always v1 (required)
  jsonValue += "\"ver\": \"v1\"";
  
  //The algorithm used to sign this file. Either "HS256" (HMAC-SHA256) or "none" (required)
  jsonValue += ",\n\"alg\" : \"none\"";
  
  //Date when the file was created in seconds since epoch. Only set this when the device creating the file has an accurate clock (optional)
  //jsonValue += ",\n\"iat\" : 0";
  
  //End of information about the signature
  jsonValue += "},\n";
  
  //Cryptographic signature for this file
  //Needs to be implemented, 64 times 0
  jsonValue += "\"signature\" : \"0000000000000000000000000000000000000000000000000000000000000000\",\n";
  
  //Sensor data, payload
  jsonValue += "\"payload\" : {\n";
  
  //Unique identifier for this device. Only set this when the device has a globally unique identifier (e.g. MAC address)
  //If this field is set the device shows up on the 'Devices' page in the studio (optional).
  //jsonValue += "\"device_name\": \"DeviceName\",\n";
  
  //Device type, for example the exact model of the device. Should be the same for all similar devices (required).
  jsonValue += "\"device_type\" : \"" + deviceTyp + "\",\n";
  
  //the frequency of the data in this file (in milliseconds). E.g. for 100Hz fill in 10 (new data every 10 ms.)
  //You can use a float here if you need the precision (required)
  jsonValue += "\"interval_ms\": " + (dataRate) + ", \n";

  //Array with sensor axes
  jsonValue += "\"sensors\": [\n";
  for (int i = 0; i < numberOfSensors; i++) {
    //Name of the axis
    jsonValue += "{ \"name\": \"";
    jsonValue += nameOfSensor[i];
    //Type of data on this axis. Needs to comply to SenML units (required).
    jsonValue += "\", \"units\": \"";
    jsonValue += unitOfSensor[i];
    if (i < numberOfSensors - 1)
    {
      jsonValue += "\" }, \n";
    }
    else {
      jsonValue += "\" } \n";
    }
  }
  jsonValue += "], \n";

  //Array of sensor values. One array item per interval, and as many items in this array as there are sensor axes
  //If you have a single sensor, you are allowed to flatten this array to save space.
  jsonValue += "\"values\": [\n";
  for (int i = 0; i < numberOfDataPoints; i++) {
    jsonValue += "[ ";
    for (int j = 0; j < numberOfSensors; j++) {
//      jsonValue += String(sensorValues[i][j]);
       jsonValue += String(sensorValues[i*numberOfSensors + j]);
      if (j < numberOfSensors - 1) {
        jsonValue += ", ";
      }
    }
    jsonValue += " ]";
    if (i < numberOfDataPoints - 1) {
      jsonValue += ", \n";
    }
    else {
      jsonValue += " \n";
    }
  }
  jsonValue += "]\n";

// if (IOTW_debug_level) Serial.println(jsonValue);
 //return
  
  //End of Payload
  jsonValue += "}\n";
  //End of JSON file
  jsonValue += "}\n";
  if (IOTW_debug_level) Serial.println(jsonValue);

  
  //Start of HTTP POST
  HTTPClient http; //Declare object of class HTTPClient
  //Connect to EdgeImpulse server http://ingestion.edgeimpulse.com
  String req = "";
  if(String("Training") == apiToLoadDataTo){
    req += "http://" + server + "/api/training/data"; //Send training data
  }else if(String("Test") == apiToLoadDataTo){
    req += "http://" + server + "/api/testing/data"; //Send test data
  }else if(String("Anomaly") == apiToLoadDataTo){
    req += "http://" + server + "/api/anomaly/data"; //Send anomaly data
  }else {
    //Wrong input parameter, upload to training api
    req += "http://" + server + "/api/training/data"; //Send training data
  }
  if (IOTW_debug_level) Serial.println(req);
  

  http.begin(wifiClient,req); //Specify request destination
  //Header needs x-api-key, x-file-name, x-label(optional)
  http.addHeader("Content-Type", "application/json"); //Required
  http.addHeader("x-api-key", apiKey); //Required
  http.addHeader("x-file-name", fileName); //Required
  //http.addHeader("x-label", ""); //Optional
  int httpCode = http.POST(jsonValue); //Send the request
  String payload = http.getString(); //Get the response payload
  if (IOTW_debug_level) Serial.println(httpCode);  //Print HTTP return code
  if (IOTW_debug_level) Serial.println(payload); //Print request response payload
  http.end(); //Close connection
  //End of HTTP POST
  
}
#endif
