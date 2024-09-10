#include <esp_now.h>
#include <WiFi.h>
#define timeSeconds 10
// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t broadcastAddress1[] = {0xB0, 0xA7, 0x32, 0x2D, 0x99, 0x1C}; //--> REPLACE WITH THE MAC Address of your receiver / ESP32 Receiver.
uint8_t broadcastAddress2[] = {0xB0, 0xA7, 0x32, 0x2D, 0x99, 0x58};
const int motionSensor = 35;
esp_err_t result;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;
typedef struct test_struct {
  int x;
  
} test_struct;

test_struct test;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void IRAM_ATTR detectsMovement() {
  test.x = 1;
  esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct));
  startTimer = true;
  lastTrigger = millis();
}
void setup() {
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
  pinMode(motionSensor, INPUT_PULLUP);
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
   
  // register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // register first peer  
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register second peer  
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
 
}
 
void loop() {
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    test.x = 0;
    esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct));
    startTimer = false;
    motion = false;
  }
}