/*

  modified October 31, 2014
  by Ken Lynch
 */
 #include <SPI.h>
#include "Adafruit_MAX31855.h"

// Thermocouple with software SPI 
#define DO   10
#define CS   11
#define CLK  12
Adafruit_MAX31855 thermocouple(CLK, CS, DO);

// the process will have three phases
// 1 - get to 150C and hold for 120 sec
// 2 - slow ramp to 183C for 100 Sec
// 3 - rapid ramp to 215C for peak event then cooldown
// all off cool down


int UpperHeat = 7;
int LowerHeat = 6;
int Motor = 5;
int other = 4;
long curMillis = 0;
long prevMillis = 0;
long curSec = 0;
int bRunning = 0;
int inCmd = 0;
int bStarted = 0;
int bReflowReached = 0;
String tempMsg = "";
//int tempReflow = 215;
//int tempPreheat = 100;
//int tempFluxActivate = 150;
//int tempSoak = 183;

int tempReflow = 110;
int tempPreheat = 50;
int tempFluxActivate = 75;
int tempSoak = 83;


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  Serial.begin(9600);
  Serial.println("MAX31855 test");

  pinMode(UpperHeat, OUTPUT);
  pinMode(LowerHeat, OUTPUT);
  pinMode(Motor, OUTPUT);
  pinMode(other, OUTPUT);
}

void stopAll()
{
      digitalWrite(UpperHeat, LOW);   
      digitalWrite(LowerHeat, LOW);   
      digitalWrite(Motor, LOW);   
      digitalWrite(other, LOW);   
}
void stopTopHeat()
{
       digitalWrite(UpperHeat, LOW);   
}
void stopLowerHeat()
{
       digitalWrite(LowerHeat, LOW);   
}
void startTopHeat()
{
       digitalWrite(UpperHeat, HIGH);   
}
void startLowerHeat()
{
       digitalWrite(LowerHeat, HIGH);   
}

// the loop function runs over and over again forever
void loop() {
  tempMsg = String("Cntl = ") + String(thermocouple.readInternal()) + String("; Oven = ");
  tempMsg += String(thermocouple.readCelsius()); 
   tempMsg += " degC / ";
    tempMsg +=  String(thermocouple.readFarenheit());
    tempMsg +=  " degF";

    Serial.println(tempMsg);

   double c = thermocouple.readCelsius();
   if (isnan(c)) {
     Serial.println("Something wrong with thermocouple!");
   } 
   Serial.print("Phase = ");
 
    if (Serial.available() > 0)
   {
     inCmd = Serial.read();  
     switch(inCmd)
     {
       case 'g':   // go
         bRunning = 1;
         break;
       case 's':   // stop
         bRunning = 0;
         stopAll();
         break;
     }
   }

   if (bRunning == 0)
   {
      stopAll();
      delay(2000);
     Serial.println("Not Running");
     return;
   }

   curMillis = millis();
   if (bStarted == 1)
      curSec = (curMillis - prevMillis) / 1000;
   else
      curSec = 0;

   if(bStarted == 0 && bRunning == 1)
   {
     Serial.println("Preheat");
     if (c < tempPreheat)
     {
        startTopHeat();
        startLowerHeat();
        delay(4000);              // wait for a second
     }
     else
     {
       bStarted = 1;
       stopLowerHeat();
        prevMillis = curMillis;  // start timing
     }
   }
   else if (curSec < 120)  // Phase 1
   {  
     Serial.println("Flux Activation");
     if (c < tempFluxActivate)
     {
        startTopHeat();
        startLowerHeat();
        delay(4000);              // wait for a second
     }
     else
     {
        stopTopHeat();
        stopLowerHeat();
        delay(1000);              // wait for a second
     }
   }
   else if (curSec >= 120 && curSec < 220)  // Phase 1
   {  
       Serial.println("Soak");
       stopLowerHeat();
     if (c < tempSoak)
     {
        startTopHeat();
        delay(4000);              // wait for a second
     }
     else
     {
        stopTopHeat();
        delay(1000);              // wait for a second
     }
   }
   else if (curSec >= 220 && bReflowReached == 0)  // Phase 1
   {  
     Serial.println("Reflow");
     if (c < tempReflow)
     {
        startTopHeat();
        startLowerHeat();
        delay(4000);              // wait for a second
     }
     else
     {
        bReflowReached = 1;
        stopAll();
        prevMillis = 0;  // Start the cooldown
     }
   }
   else 
   {
      Serial.println("Cooldown");
     stopAll();
     bRunning = 0;
     bStarted = 1;
     delay(1000);              // wait for a second
   } 
 
}
