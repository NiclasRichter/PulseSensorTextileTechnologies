/* Sensor-Unit */
 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define analogPin A0                        // Pin for reading sensorsignal S(t)

int       PS_value          =0;             // PulseSensor-Value from analogPin
const int diffOrder         =1;             // Order of discretization for further developments
int       PS_signal[diffOrder+1];           // Array for calculating the derivative
int       delta_t_loop      =20;            // Loop-delay in ms
int       millimin          =60000;         // Milliseconds per minute
double    alpha             =570.0;         // Threshold for C_1, alpha
double    beta              =0.2;           // Threshold for C_2, beta
double    dt_PS_signal;                     // Rate of change for Criteria C_2
int       n_loop            =0;             // Loops between two heartbeats
int       delta_t_HB        =1000;          // Time between two heartbeats
double    avgBPM            =0.0;           // Average beats per minute
const int n_samples         =5;             // Number of samples for calculating average BPM
double    delta_t_HB_Arr[n_samples];        // Storage for calculating average BPM 
double    sum;                              // Summed up timeBetweenHeartbeats for averaging

uint8_t peer1[] = {0x24, 0x0A, 0xC4, 0x5A, 0x05, 0x74}; // Define the Receiver-Unit via MAC-adress

typedef struct message {
  int PS_value;
  double avgBPM;
};
struct message DataPkg;
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Get Mac Add
  Serial.print("Mac Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println("ESP-Now Sender");

  if (esp_now_init() != 0){
    Serial.println("Problem during ESP-NOW init");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(peer1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {

PS_value = analogRead(analogPin);                                  // Read analogvalue from Sensor and store it in S(t)
PS_signal[0]=PS_signal[1];                                         // Push data through array with 2 elements
PS_signal[1]=PS_value;						   // -> 2, because order of differencing scheme is one
dt_PS_signal=1.0*(PS_signal[1]-PS_signal[0])/delta_t_loop;         // Backward differencing scheme first order

if (n_loop != 0 && PS_value>=alpha && dt_PS_signal>=beta){         // Detect the heartbeat with the presented Criteria C_1&2         
  delta_t_HB=(double)n_loop*(double)delta_t_loop;                  // If heartbeat detected, calculate time between heartbeat

  for(int i=0;i<n_samples-1;i++){                                  // Push data through array, until second last element
    delta_t_HB_Arr[i]=delta_t_HB_Arr[i+1];
  }

  if(delta_t_HB>350){                                              // Little bit of cosmetics to the calculation of the meanvalue
   delta_t_HB_Arr[n_samples-1]=delta_t_HB;                         // If time between two heartbeats is long enough, take it for the calculation    
  }                                                                //
  else{delta_t_HB_Arr[n_samples-1]=delta_t_HB_Arr[n_samples-2];}   // If time ist too short, take the last plausible one

  sum=0.0;                                                         // Sum for calculation mean-value
  for(int i=0;i<=(int)n_samples-1;i++){                            // Add up the whole array
    sum=sum+delta_t_HB_Arr[i];
  }

  avgBPM=1.0*n_samples*millimin/sum;                               // Calculate avgBPM
  n_loop=0;                                                        // Set n_loop to 0 again 
}
else {                                                             // Count up number of loops to calculate time between heartbeats
  n_loop++;
}

DataPkg.PS_value = PS_value;                                       // Construct DataPkg 
DataPkg.avgBPM = avgBPM;
esp_now_send(NULL, (uint8_t *) &DataPkg, sizeof(DataPkg));         // Send DataPkg

delay(delta_t_loop);
}


