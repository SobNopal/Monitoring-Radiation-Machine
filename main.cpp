// //include library
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define clickPin 19

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//configure WiFi Connected
char SSID[] = "Your SSID"; //your name Wifi
char PASS[] = "Your Password";//your password Wifi

const char *apiEndpoint = "http://192.168.1.8:8000/api/sensors"; // Replace this with your API endpoint

// int awaitTime = 1000;
int clicks;
bool firstClick = false;
int CPM;
float uSv;
//float roentgen;
float MAXIMUM_TOLERANCE = 7.0;

void onClick();
void displayToPlotter();
void toSievert();
void wait();
void redAlert();
void showCPM();

unsigned long previousMillis=0;
unsigned long sendMillis=0;
unsigned long currentMillis = millis();

void setup() {
  attachInterrupt(digitalPinToInterrupt(clickPin), onClick, FALLING);
  digitalWrite(clickPin, LOW);
  Serial.begin(9600);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  +Serial.println("");
  Serial.println("WiFi connected");

  delay(500); 
  oled.setCursor(0, 0);

  Serial.println("Awaiting random click from Geiger on Input pin...");
  while (firstClick == false){
    Serial.print("...");
  }
  //TEST
  Serial.println();
  Serial.print("Success after:");
  Serial.println(String(millis()) + "ms");
  Serial.println();
}

void onClick(){
  firstClick = true;
  clicks++;
}

void toSievert(){
  uSv = CPM * 0.00812037037037; //perhitungan convert dari nilai cpm ke nilai mikrosieviet/hour
  Serial.println();
  Serial.print("Current µSv/h: ");
  Serial.print(uSv);
  Serial.println(" µSv/h");
  Serial.println();
  if (uSv >= MAXIMUM_TOLERANCE){
    redAlert();
  }
}

// Function to perform CRUD operations
String performCRUD(const char* method, DynamicJsonDocument payload) {
  HTTPClient http;

  http.begin(apiEndpoint);

  // Add headers
  http.addHeader("Content-Type", "application/json");

  // Convert payload to JSON string
  String jsonString;
  serializeJson(payload, jsonString);
  Serial.println(jsonString);

  // Make the request
  int httpResponseCode;
  if (method == "POST") {
    httpResponseCode = http.POST(jsonString);
  }

  // Check for errors
  if (httpResponseCode > 0) {
    String response = http.getString();
    http.end();
    return response;
  } else {
    Serial.println("HTTP request failed");
    http.end();
    return "";
  }
}

void senddata() {
  configTime(0, 0, "pool.ntp.org");  // Set NTP time server

  Serial.println("Waiting for NTP time sync");
  while (time(nullptr) < 1609468800) {
    delay(500);
    Serial.print(".");
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Tambahkan offset zona waktu
  timeinfo.tm_hour += 7;

  char formattedTime[20];  // Adjust the buffer size as needed
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

  Serial.println(formattedTime);

  // Example: Create new data entry
  DynamicJsonDocument createData(200);
  createData["ts_tanggal_input"] = formattedTime;   // Include the formatted time string
  createData["ts_cpm"] = CPM;                       // Make sure 'CPM' is defined and has a value
  createData["ts_radiation"] = uSv;                 // Make sure 'Radiation' is defined and has a value

  String createResponse = performCRUD("POST", createData);
  Serial.println(createResponse);
}

void loop() {
  showOled();
  radiation();
  if (millis() - sendMillis > 60000) {
    senddata();
    sendMillis = millis();
  }
}

void radiation(){
  previousMillis = currentMillis;
  while (currentMillis - previousMillis <= 500){
    currentMillis = millis();
  }
  CPM = clicks;
  clicks = 0;
  Serial.println("-------------------RESULTS OF THE LAST MINUTE-------------------");
  showCPM();
  toSievert();
  Serial.println("----------------------------------------------------------------");
}

void redAlert(){
  Serial.println();
  Serial.print("The highest tolerance of, ");
  Serial.print(MAXIMUM_TOLERANCE);
  Serial.print(" µSv/h has been reached!");
  Serial.println();
  Serial.println("LEAVE THE AREA IMMEDIATELY!");
}

void showCPM(){
  Serial.println();
  Serial.print("Current CPM: ");
  Serial.println(CPM);
  Serial.println();
}

void showOled(){
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

          // wait for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(2);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(8, 5);        // position to display
  oled.print("CPM : "); // text to display
  oled.println(CPM); // text to display
  oled.setTextSize(2);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(3, 35);        // position to display
  oled.print(uSv); // text to display
  oled.println(" µSv/h"); // text to display
  oled.drawRect(0, 25, 128, 30, WHITE);
  oled.display(); 
}
