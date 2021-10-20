//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const int potpin = 34;

BluetoothSerial SerialBT;
/*
//Filtering method 1_Averaging
// Smoothing, value to determine the size of the readings array.
const int numReadings = 2;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0.0;              // the index of the current reading
float total = 0.0;                  // the running total
float average = 0.0;                // the average
float data = 0.0;                   //sent data
int inputPin = A5;
*/

//Filtering method 2_IIR Filtering
float b0=1 ; //IIR Filter Coeficients
float b1=0 ;
float b2=1 ;
float a1=0 ;
float a2=0.9882 ;
float x=0;//signal variable declaration
float x1=0.0;
float x2=0.0;
float y=0.0;
float yl =0.0;
float y2=0.0;
int datain[50];//ECG data initialization
int dataout[50];
int j=0;

//Filtering Method3_Weight
#include "Filter.h"
// Create a new exponential filter with a weight of 50 and an initial value of 0. (0%-100%) 0%=More smoothing 100% = No smoothing
ExponentialFilter<float> Weight_Filter(55, 0);
float weighted_data = 0;

//BPM int's
int UpperThreshold = 450;
int LowerThreshold = 400;
float readingBPM = 0;
int BPM = 0;
bool IgnoreReading = false;
bool FirstPulseDetected = false;
unsigned long FirstPulseTime = 0;
unsigned long SecondPulseTime = 0;
unsigned long PulseInterval = 0;
const unsigned long delayTime = 10;
const unsigned long delayTime2 = 5000;
const unsigned long baudRate = 9600;
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;


//Make data faster
int a[4];
int i = 0;
int oldmillis = 0;

void setup() {
  pinMode(12, INPUT); // Setup for leads off detection LO +
  pinMode(13, INPUT); // Setup for leads off detection LO -
  Serial.begin(115200);
 
  delay(2000); // wait for bluetooth module to start
  SerialBT.begin("ESP32test"); //Bluetooth device name
}

/*
  // Smoothing, initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  */




void loop() {

/*
//Filter 1 Averaging
// Smoothing, subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = dataout[j];
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
*/


  //Filter 2 IIR
  datain[j]=analogRead(potpin)/6;
  x=datain[j];
  y=b0*x + b1*x1 + b2*x2 - a1*yl - a2*y2;
  y2=yl;
  yl=y;
  x2=x1;
  x1=x;
  dataout[j] = y;
  //Serial.println(dataout[j]);
  j=j+1;
if (j>49)
{ 
  j=0;
}

//Filter 3 Weight
Weight_Filter.Filter(dataout[j]);
weighted_data = Weight_Filter.Current();
  

  /*if((digitalRead(12) == 1)||(digitalRead(11) == 1)){
    Serial.println('!');
  }
  else{
    // send the value of analog input 0:
      bluetooth.print( (String)"A5 " + (average*1.5-170) + "\n" );
  }*/
 if (millis() - oldmillis >= 10)
         {
               oldmillis = millis();
                    a[i] = weighted_data;
                    //a[i] = analogRead(inputPin);
                       i = i + 1;
               if(i > 3)
               {
                         i = 0;
                         SerialBT.print(a[0]); 
                         SerialBT.print('|'); 
                         SerialBT.print(a[1]); 
                         SerialBT.print('|');
                         SerialBT.print(a[2]); 
                         SerialBT.print('|');
                         SerialBT.print(a[3]);
                         SerialBT.print('|');
                         SerialBT.println(BPM);
                }
       }
 // bluetooth.println(BPM,0);
  delay(10);


// Get current time
  unsigned long currentMillis = millis();


  // First event
  if(myTimer1(delayTime, currentMillis) == 1){

    readingBPM = analogRead(potpin)/5; 

    // Heart beat leading edge detected.
    if(readingBPM > UpperThreshold && IgnoreReading == false){
      if(FirstPulseDetected == false){
        FirstPulseTime = millis();
        FirstPulseDetected = true;
      }
      else{
        SecondPulseTime = millis();
        PulseInterval = SecondPulseTime - FirstPulseTime;
        FirstPulseTime = SecondPulseTime;
      }
      IgnoreReading = true;
      //digitalWrite(LED_BUILTIN, HIGH);
    }

    // Heart beat trailing edge detected.
    if(readingBPM < LowerThreshold && IgnoreReading == true){
      IgnoreReading = false;
     //digitalWrite(LED_BUILTIN, LOW);
    }  

    // Calculate Beats Per Minute.
    BPM = (1.0/PulseInterval) * 60.0 * 1000;
  }

 
/*
  // Second event
  
  if(myTimer2(delayTime2, currentMillis) == 1){
    //Serial.print(reading);
    //Serial.print("\t");
    //Serial.print(PulseInterval);
    //Serial.print("\t");
    //Serial.print(BPM);
    //Serial.println(" BPM");
    //Serial.flush();
    bluetooth.print( (String)"BPM_rate " + BPM + "\n");
  }*/
  
}

// First event timer
int myTimer1(long delayTime, long currentMillis){
  if(currentMillis - previousMillis >= delayTime){previousMillis = currentMillis;return 1;}
  else{return 0;}
}

// Second event timer
int myTimer2(long delayTime2, long currentMillis){
  if(currentMillis - previousMillis2 >= delayTime2){previousMillis2 = currentMillis;return 1;}
  else{return 0;}

  }
