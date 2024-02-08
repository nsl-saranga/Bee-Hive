// ####################################################################################
// # main program #####################################################################
// ####################################################################################
#ifdef release

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HX711.h>
// #include <arduinoFFT.h>
#include <ArduinoJson.h>

#define DHTTYPE DHT22
#define DHT22PIN 2

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHT22PIN, DHTTYPE);

// HX711 circuit wiring
const int LOADCELL_DO_PIN = 3;
const int LOADCELL_CK_PIN = 4;

HX711 scale;

// API endpoint details
// `API_KEY, API_SERVER, API_PATH, API_PORT` are defined

// GSM auth details
// `GSM_APN, GSM_USER, GSM_PASSWORD` are defined
SoftwareSerial SIM900(7, 8);//Rx,TX

void setup()
{
  Serial.begin(9600);
  dht.begin();
  scale.begin(LOADCELL_DO_PIN, LOADCELL_CK_PIN);
  scale.set_scale(0); // <<<<<<<<<<<<<<<<<< replace with the "Calibration factor" value
  scale.tare();
  SIM900.begin(9600);
  // Initialize GPRS connection
  initializeGPRS();
}

void loop()
{
    // Power up the ADC
    scale.power_up();
    // Read data and store it to variables temp, humidity, and weight
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    float weight = scale.get_units(10);
  
    // Create JSON payload
    String jsonPayload = "{\"temperature\":" + String(temp) +
                        ",\"humidity\":" + String(humidity) +
                        ",\"weight\":" + String(weight) + "}";


    // Send data to the API
    sendDataToAPI(jsonPayload);
    // Power down the ADC
    scale.power_down();
    delay(10000);
}

void initializeGPRS() {
  SIM900.println("AT");
  delay(1000);
  SIM900.println("AT+CPIN?");
  delay(1000);
  SIM900.println("AT+CREG?");
  delay(1000);
  SIM900.println("AT+CGATT?");
  delay(1000);
  SIM900.println("AT+CIPSHUT");
  delay(1000);
  SIM900.println("AT+CIPSTATUS");
  delay(1000);
  SIM900.println("AT+CIPMUX=0");
  delay(1000);
  SIM900.print("AT+CSTT=\"");
  SIM900.print(GSM_APN);
  SIM900.println("\"");
  delay(1000);
  SIM900.println("AT+CIICR");
  delay(3000);
  SIM900.println("AT+CIFSR");
  delay(2000);
}

void sendDataToAPI(String data) {
   SIM900.println("AT+CIPSTART=\"TCP\",\"" + String(API_SERVER) + "\"," + String(API_PORT));
  delay(2000);
  if (SIM900.find("OK")) {
    Serial.println("TCP Connection Established");
  } else {
    Serial.println("Error: Unable to establish TCP connection");
    return;
  }

  String postRequest =
      "POST " + String(API_PATH) + " HTTP/1.1\r\n" +
      "Host: " + String(API_SERVER) + "\r\n" +
      "Content-Type: application/json\r\n" +
      "Content-Length: " + String(data.length() + 2) + "\r\n" +
      "Authorization: Basic " + String(GSM_APN) + "\r\n" +
      "\r\n" +
      data;

  SIM900.print("AT+CIPSEND=");
  SIM900.println(postRequest.length());
  delay(1000);
  if (SIM900.find(">")) {
    Serial.println("Sending data...");
    SIM900.print(postRequest);
  } else {
    Serial.println("Error: Unable to send data");
  }

  delay(5000); // Wait for the server response (adjust accordingly)

  // Close the connection
  SIM900.println("AT+CIPCLOSE");
}
#endif

// ####################################################################################
// # calibration for the HX711 module #################################################
// ####################################################################################
#ifdef calibrate
#include <HX711.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 4;

HX711 scale;

void setup()
{
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop()
{
  // keep the loop delay up here so there is a delay before the first iteration.
  delay(1000);
  if (scale.is_ready())
  {
    Serial.println("HX711 found.");
    scale.set_scale();
    Serial.println("Tare... remove any weights from the scale.");
    delay(5000);
    scale.tare();
    Serial.println("Tare done...");
    Serial.println("Place a known weight on the scale... you have 10 seconds");
    delay(10000);
    long reading = scale.get_units(10);
    Serial.print("Reading: ");
    Serial.println(reading);
    Serial.print("Calibration factor will be the (reading)/(known weight).");
    Serial.println("Replace this value in main program. Unit of the known weight will be used for weight reading in the main program.");
  }
  else
  {
    Serial.println("HX711 not found.");
  }
}
#endif
