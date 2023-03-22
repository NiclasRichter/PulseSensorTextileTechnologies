/* Receiver-Unit */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>                   
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();

typedef struct message 
{
  int PS_Value;     			// S(t)
  double avgBPM;    			// BPM_mean
};
struct message DataPkg;     

int PS_Value; 
double avgBPM;                          
        
const int displayLength = 160;          // Length of Display in Pixel
const int displayHeight = 128;          // Height of Display in Pixel
int graph[displayLength];               // Storage for Plotting S(t)

void onDataReceiver(const uint8_t * mac, const uint8_t *incomingData, int len) // Do this {...}, when receiving DataPkg
{
 memcpy(&DataPkg, incomingData, sizeof(DataPkg));

 PS_Value=( DataPkg.PS_Value);      // Assign the Elements of the pakage to the variables
 avgBPM=(DataPkg.avgBPM); 

for(int i=0;i<displayLength-1;i++){                       // "Push the Data through the Array"
   graph[i]=graph[i+1];                                   // First element gets value from second, second from third, ...
}                                                         // Until second last element, 
graph[displayLength-1]=map(PS_Value,500,650,128,0);       // because last one gets the value mapped from DataPkg / S(t)

tft.fillScreen(TFT_WHITE);                                // Reset screen to white
tft.drawString("avgBPM:", 5, 5, 2);                       // Draw BPM_mean string on display
tft.drawNumber(avgBPM, 70, 5, 2);                         // Draw BPM_mean value on display

for(int i=0;i<displayLength-1;i++){                       // Draw black line from one coordinate to another           
   tft.drawLine(i,graph[i],i+1,graph[i+1], TFT_BLACK);    // Coordinates are the single S(t)-Values, which are mapped to
  }                                                       // the pixelrange of the Display and stored in graph[i]
}

void setup() {
Serial.begin(115200);
WiFi.mode(WIFI_STA);
// Get Mac Add
Serial.print("Mac Address: ");
Serial.print(WiFi.macAddress());
Serial.println("\nESP-Now Receiver");

if (esp_now_init() != 0) {                                // Initializing ESP-NOW
  Serial.println("Problem during ESP-NOW init");
  return;
  }

tft.setTextColor(TFT_BLACK, TFT_WHITE);
tft.init();
tft.setRotation(1);
tft.fillScreen(TFT_WHITE);

for(int i=0;i<displayLength;i++){                        // Initialize graph[i] as straight line in the middle of the display
   graph[i]=64;
  }

for(int i=0;i<displayLength;i++){                        // Draw line 
   tft.drawPixel(i,graph[i], TFT_BLACK);
  }
tft.drawString("avgBPM: No Signal!", 5, 5, 2);
esp_now_register_recv_cb(onDataReceiver);
}
 
void loop() {
}
