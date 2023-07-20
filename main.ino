/*
  based on 
  https://RandomNerdTutorials.com/esp32-esp8266-mysql-database-php/

  https://github.com/RuiSantosdotme/ESP32-ESP8266-PHP-MySQL/blob/master/code/HTTPS_ESP32_MySQL_Database_PHP.ino

  max30100 examples  

                ESP32
          +----------------+
          |                |
          |              22|<-- SCL MAX30100
          |                |
          |              21|<-- SDA MAX30100
          |                |
          |                |
          +----------------+  

  last update: may 1st, 2023. Not tested with web server
  todo: 
  [x] check it compiles
  [x] upload to board, seems to be working
  [x] check request succesfully done
  [ ] check sensor successfully reading, always reading zeros
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Replace with your network credentials
// const char* ssid     = "INFINITUM6830";
// const char* password = "FYvBJDhAUj";
const char* ssid     = "REIKA-2";
const char* password = "REIKA3110";

// REPLACE with your Domain name and URL path or IP address with path
// const char* serverName = "http://192.168.1.66/_experiments/esp_32/post_insert.php";
const char* serverName = "http://192.168.0.22/_experiments/esp_32/post_insert.php";
// C:\wamp64\www\_experiments\esp_32\post_insert.php

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";

// String sensorName = "BME280";
// String sensorLocation = "Office";

PulseOximeter pulse_oximeter;
float pulse_oximeter_values[2];

#define ADC_VREF_mV    3300.0 // in millivolt
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       36 // ESP32 pin GIOP36 (ADC0) connected to LM35

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  setup_pulse_oximeter();
}

void loop() {
    pulse_oximeter.update();  
    read_sensor();
    Serial.print("H:");
    Serial.println(pulse_oximeter_values[0]);

    Serial.print("O:");
    Serial.println(pulse_oximeter_values[1]);

    float tempC = read_temperature_sensor();
  
  if (!(WiFi.status() == WL_CONNECTED)) {
    Serial.println("WiFi Disconnected");
  } else {
    HTTPClient request;
    
    request.begin(serverName);
    request.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor_names=" + "MAX30100"
                          + "&temperature=" + String(tempC)
                          + "&heart_rate=" + String(pulse_oximeter_values[0]) 
                          + "&oxygen_saturation=" + String(pulse_oximeter_values[1]);
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // Send HTTP POST request
    int httpResponseCode = request.POST(httpRequestData);
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    request.end();      
  }

  //Send an HTTP POST request every 30 seconds
  delay(30000);
}
//--------------------------------------------------------------------------------------
// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}
/**
* Initializes the MX30100 sensor
*/
void setup_pulse_oximeter()
{
    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pulse_oximeter.begin()) {
        Serial.println("FAILED");
        // for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pulse_oximeter.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pulse_oximeter.setOnBeatDetectedCallback(onBeatDetected);
}
/**
*
*/
void read_sensor()
{
  // float sensor_readings[2];
  pulse_oximeter_values[0] = pulse_oximeter.getHeartRate();
  pulse_oximeter_values[1] = pulse_oximeter.getSpO2();
  // return sensor_readings;
}

float read_temperature_sensor()
{
  // read the ADC value from the temperature sensor
  int adcVal = analogRead(PIN_LM35);
  // convert the ADC value to voltage in millivolt
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  // convert the voltage to the temperature in °C
  float tempC = milliVolt / 10;
  // convert the °C to °F
  float tempF = tempC * 9 / 5 + 32;

  // print the temperature in the Serial Monitor:
  // Serial.print("Temperature: ");
  // Serial.print(tempC);   // print the temperature in °C
  // Serial.print("°C");
  // Serial.print("  ~  "); // separator between °C and °F
  // Serial.print(tempF);   // print the temperature in °F
  // Serial.println("°F");
  return tempC;
}
