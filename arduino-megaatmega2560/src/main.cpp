// ####################################################################################
// # main program #####################################################################
// ####################################################################################
#ifdef release

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HX711.h>
#include <arduinoFFT.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define DHTTYPE DHT22
#define DHT22PIN 2

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHT22PIN, DHTTYPE);

// HX711 circuit wiring
const int LOADCELL_DO_PIN = 3;
const int LOADCELL_CK_PIN = 4;

HX711 scale;

// Define the analog pin connected to the MAX4466
const int analogPin = A0;
arduinoFFT fft = arduinoFFT();

// for the FFT algo
#define SAMPLES 128 // Must be a power of 2
#define SAMPLING_FREQUENCY 10000  // Hz, must be less than 10000 due to ADC limitations

// Defines the interval (in milliseconds) at which you want to process the accumulated data.
#define Prcoess_INTERVAL 5*60*1000  // 5 mins in milliseconds
// Defines the interval (in milliseconds) at which you want to sample data (call perform_FFT()).
#define FFT_SAMPLING_INTERVAL 1*60*1000  // 1 min in milliseconds

//  Keeps track of the time (in milliseconds) when the last FFT processing occurred.
unsigned long lastProcessTime = 0; //  Keeps track of the time (in milliseconds) when the last FFT processing occurred.
unsigned long lastFFTTime = 0; // Keeps track of the time (in milliseconds) when the last data sampling (calling perform_FFT()) occurred.
float accumulatedData[5];  // Array to accumulate data over 5 seconds
int dataIndex = 0;  // Index for the accumulatedData array

// Variables for FFT
double vReal[SAMPLES];
double vImag[SAMPLES];

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
  unsigned long currentTime = millis();
  // Calculate the time elapsed since the last FFT operation and the last data sampling, respectively.
  unsigned long elapsedTimeProcess = currentTime - lastProcessTime;
  unsigned long elapsedTimeFFT = currentTime - lastFFTTime;


  // if block checks if the elapsed time since the last data sampling is greater than or equal to the specified sampling interval 
  if (elapsedTimeFFT >= FFT_SAMPLING_INTERVAL) {
    // Code inside this block runs every second
    // Call your user-defined function for FFT
    float sec_frequency = perform_FFT();
    accumulatedData[dataIndex] = sec_frequency;
    dataIndex++;
    
  
    // Update the last FFT time
    lastFFTTime = currentTime;
  }

  if (elapsedTimeProcess >= Prcoess_INTERVAL) {
    // Code inside this block runs every 5 seconds

    // Process the accumulated data here
    // ...
    
    // Power up the ADC
    scale.power_up();
    // Read data and store it to variables temp, humidity, and weight
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    float weight = scale.get_units(10);
    float frequencies[5] = {accumulatedData[0], accumulatedData[1], accumulatedData[2], accumulatedData[3], accumulatedData[4]};
    // Create JSON payload
    String jsonPayload = "{\"temperature\":" + String(temp) +
                        ",\"humidity\":" + String(humidity) +
                        ",\"weight\":" + String(weight) +
                        ",\"frequencies\":[";
                        
    // Add frequencies to the JSON payload
    for (int i = 0; i < 5; ++i) {
        jsonPayload += String(frequencies[i]);
        if (i < 4) {
            jsonPayload += ","; // Add a comma except for the last element
        }
    }
    jsonPayload += "]}";

    // Send data to the API
    sendDataToAPI(jsonPayload);

    if(dataIndex>=5)
    {
      memset(accumulatedData, 0, sizeof(accumulatedData));
      dataIndex = 0;
    }
    // Power down the ADC
    scale.power_down();
   

    // Update the last FFT time
    lastProcessTime = currentTime;
  }
}

float perform_FFT(){
  // Perform FFT on the analog signal
  fft.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  fft.ComplexToMagnitude(vReal, vImag, SAMPLES);
  
  // Find the index of the peak in the frequency domain
  int peakIndex = 0;
  double maxMagnitude = 0.0;
  
  for (int i = 1; i < SAMPLES / 2; i++) {
    // int mic = analogRead(microphonePin);
    if (vReal[i] > maxMagnitude) {
      maxMagnitude = vReal[i];
      peakIndex = i;
    }
  }

  // Calculate the frequency corresponding to the peak index
  float frequency = peakIndex * (SAMPLING_FREQUENCY / 2.0) / SAMPLES;
  return frequency;
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
  SIM900.println("AT+CSTT=\"GSM_APN\"");
  delay(1000);
  SIM900.println("AT+CIICR");
  delay(3000);
  SIM900.println("AT+CIFSR");
  delay(2000);
}

void sendDataToAPI(String data) {
  SIM900.println("AT+CIPSTART=\"TCP\",\"" + String(API_SERVER) + "\",80");
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

  SIM900.println("AT+CIPSEND=" + String(postRequest.length()));
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
