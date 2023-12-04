// ####################################################################################
// # main program #####################################################################
// ####################################################################################
#ifdef release

#include <Arduino.h>

#include <DHT.h>
#include <DHT_U.h>

#include <HX711.h>

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

void setup()
{
  Serial.begin(9600);
  dht.begin();
  scale.begin(LOADCELL_DO_PIN, LOADCELL_CK_PIN);
  scale.set_scale(0); // <<<<<<<<<<<<<<<<<< replace with the "Calibration factor" value
  scale.tare();
}

void loop()
{
  // keep the loop delay up here so there is a delay before the first iteration.
  delay(5000);
  // power up the ADC
  scale.power_up();

  // Read data and store it to variables hum and temp
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  float weight = scale.get_units(10);

  // put the ADC in sleep mode
  scale.power_down();
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
