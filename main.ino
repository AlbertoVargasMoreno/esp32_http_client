/*
                ESP32
          +----------------+
          |                |
          |              22|<-- SCL MAX30100
          |                |
          |              21|<-- SDA MAX30100
          |                |
          |                |
          +----------------+  

  last update: july 2, 2023. Already Tested with web server
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

// const char* ssid     = "INFINITUM6830";
// const char* password = "FYvBJDhAUj";
const char* ssid     = "REIKA-2";
const char* password = "REIKA3110";

// const char* serverName = "http://192.168.1.66/_experiments/esp_32/post_insert.php";
const char* serverName = "http://192.168.0.22/_experiments/esp_32/post_insert.php";

String apiKeyValue = "tPmAT5Ab3j7F9";

PulseOximeter pulse_oximeter;
float pulse_oximeter_values[2];

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
  
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient https;
    
    https.begin(serverName);
    
    // Specify content-type header
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor_names=" + "MAX30100"
                          + "&temperature=" + String(0)
                          + "&heart_rate=" + String(pulse_oximeter_values[0]) 
                          + "&oxygen_saturation=" + String(pulse_oximeter_values[1]);
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // You can comment the httpRequestData variable above
    // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
    // String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensors_names=max30100&temperature=24.75&heart_rate=49.54&oxygen_saturation=1005.14";
    // Serial.print("httpRequestData: ");
    // Serial.println(httpRequestData);

    // Send HTTP POST request
    int httpResponseCode = https.POST(httpRequestData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    https.end();
  }
  else {
    Serial.println("WiFi Disconnected");
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
        for(;;);
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
