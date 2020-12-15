#include "SparkFunLSM6DS3.h"
#include "Wire.h"
#include "WiFiEsp.h"
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11); // RX, TX
#endif

const int buttonPin = 5;
const int soundPin = A0;
const int buzzerPin = 3;

LSM6DS3 myIMU;
int min_x = 1000;
int max_x = -1000;
int min_y = 1000;
int max_y= -1000;
int buttonState = false;

int moveThreshold = 2;
long soundThreshold = 550;

char ssid[] = "Mark"; // your network SSID (name)
char pass[] = "33470095"; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status
char server[] = "18.224.64.1";
char state[10];
char get_request[200];
// Initialize the Ethernet client object
WiFiEspClient client;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
  // initialize serial for ESP module
    Serial1.begin(115200);
    WiFi.init(&Serial1);
    myIMU.begin();
    pinMode(buttonPin, INPUT);
    pinMode(buzzerPin, OUTPUT);

     if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
    }
    // attempt to connect to WiFi network
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(ssid, pass);
    }
    Serial.println("You're connected to the network");
    printWifiStatus();
    
    Serial.println("Starting connection to server...");
    client.connect(server, 5000);
    
    strncpy( state, "UNLOCKED", sizeof(state) );
    sprintf(get_request,"GET /?state=%s HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", state);
    delay(500);
    Serial.println(get_request);
    client.print(get_request);
    delay(500);
}

void loop() {
  Serial.println();

  Serial.println("Connected to server");
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  
  // put your main code here, to run repeatedly:
  if (digitalRead(buttonPin) == HIGH){
    buttonState = !buttonState;
    if(buttonState){
      strncpy( state, "LOCKED", sizeof(state) );
    } else{
      digitalWrite(buzzerPin, LOW);
      strncpy( state, "UNLOCKED", sizeof(state) );
    }
    sprintf(get_request,"GET /?state=%s HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", state);
    Serial.println(get_request);
    if (!client.connected()){
      Serial.println("Starting connection to server...");
       client.connect(server, 5000);
    }
    client.print(get_request);
  }
  Serial.println(buttonState);
  if (buttonState) { // Door is locked 
    Serial.println("locked");
    // Movement
    float x = myIMU.readFloatAccelX();
    float y = myIMU.readFloatAccelY();
    float rms = 10 * (sqrt((x * x) + (y * y)));
    Serial.println(rms);
    delay(100);
    // Sound
    long sum = 0;
    for(int i=0; i<32; i++)
    {
        sum += analogRead(soundPin);
    }
    sum >>= 5;
    Serial.println(sum);
    delay(10);
    if (strcmp(state, "ALERT") != 0) {
      if (sum > soundThreshold || rms >  moveThreshold) {
      digitalWrite(buzzerPin, HIGH);
      strncpy( state, "ALERT", sizeof(state) );
      sprintf(get_request,"GET /?state=%s HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", state);
      Serial.println(get_request);
      if (!client.connected()){
        Serial.println("Starting connection to server...");
         client.connect(server, 5000);
      }
      client.print(get_request);
      }
    }
  }
  else { // Door is unlocked
    Serial.println("not locked");
  }
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  delay(1000);
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
