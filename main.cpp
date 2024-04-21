#include <Arduino.h>
#include <ThingerESP32.h>
#include <WiFi.h>
#include <SPI.h>

//Configure Thinger.Io
//example : 
//#define USERNAME "naufal_la" //username awal masuk
//#define DEVICE_ID "Detector_Radiasi" //Device ID Thinger IO saat pembuatan device
//#define DEVICE_CREDENTIAL "pcXgC7PDVgn8dcx9" //Device Credential jangan lupa di copy dulu 

#define USERNAME ""
#define DEVICE_ID ""
#define DEVICE_CREDENTIAL ""

/*Setup Pin to Microcontroller*/
#define LEDPin 22
#define GMTube 19

#define LOG_PERIOD 15000     //Logging period in milliseconds, recommended value 15000-60000.
#define MAX_PERIOD 60000    //Maximum logging period

unsigned long counts;             //variable for GM Tube events
unsigned long cpm;                 //variable for CPM
unsigned int multiplier;             //variable for calculation CPM in this sketch
unsigned long previousMillis;      //variable for time measurement

//variable Thinger.Io
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

//configure WiFi Connected
char SSID[] = ""; //your name Wifi
char PASS[] = "";//your password Wifi

void impulse() { // Calling Signal FALLING in pin 19
    counts++;
}

void setup() {
  counts = 0;
  cpm = 0;
  multiplier = MAX_PERIOD/LOG_PERIOD;  //calculating multiplier, depend on your log period
  Serial.begin(9600);
  pinMode(GMTube, INPUT);  
  attachInterrupt(digitalPinToInterrupt(2), impulse, FALLING); //define external interrupts
  Serial.println("Start counter");
  //Connected WiFi
    WiFi.begin(SSID, PASS);
    //Check WiFi Connected
    while(WiFi.status()!= WL_CONNECTED)
    {
      digitalWrite(LEDPin,LOW);
      delay(500);
    }
    //WiFi Connected 
    digitalWrite(LEDPin,HIGH);
    //Connected ESP32 to Thinger.Io
    thing.add_wifi(SSID,PASS);

    //send data sensor GT to Thinger.Io
    thing["Radiasi"] >> [](pson & out){
      out["Nilai"] = cpm;
    };
}

void loop() {
 unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > LOG_PERIOD) {
        previousMillis = currentMillis;
        cpm = counts*multiplier;
        Serial.println(cpm);// send cpm data to Radiation Logger
        counts = 0;
    }
    thing.handle();
}
