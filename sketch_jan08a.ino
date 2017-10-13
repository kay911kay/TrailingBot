#include "DHT.h"
#include <Wire.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>
LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
#include "DHT.h"
#define trigPinFront 12 //defines the pins for the sensor on the front
#define echoPinFront 13
#define trigPinLeft 11 //defines the pins for the sensors on the left
#define echoPinLeft 10

//Variable declarations
byte wheelRf = 50; //pin for moving right wheel forward
byte wheelRb = 53; //pin for moving right wheel backward
byte wheelLf = 22; //pin for moving left wheel forward
byte wheelLb = 24; //pin for moving left wheel backward
long durationFront, distanceFront, durationLeft, distanceLeft; //these variables used to keep track of distance an object is from sensor and how often we scan
boolean objectFront = false, objectLeft = false; //variables keep track of whether or not an object is seen infront of sensor
int maxdistance = 40, mindistance = 10, oldtime = 0, value, emergencyStopTime = 0, x = 0; //variables keep track of how far the sensor will read up to, and follow up to. 
unsigned long time; //initiates a unsigned long variable since millis increases at an incredible pace


//Setup the initial environment variables
void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2); //starts the 16x2 lcd display
    pinMode(trigPinFront, OUTPUT); //initiates the transmitter portion of the ultrasonic sensors
    pinMode(echoPinFront, INPUT); //initiates the reciever portion of the sensors
    pinMode(trigPinLeft, OUTPUT);
    pinMode(echoPinLeft, INPUT);
}


//A while(1) loop, as robot needs to be constantly recieving information and making adjustments
void loop() {
    time = millis(); //starts a timer called millis
    sensorRead(); //initiates the method sensorRead
    sensorLogic(); //initiates the method sensorLogic
    motorLogic(); //initiates the method motorLogic
    processingReader(); //initiates processingReader
    if ((time - oldtime) > 3000) { //refreshes the lcd every 3 seconds to see what the robot is doing
        oldtime = time;
        lcd.clear();
    }

}

void sensorRead() { //reads from the sensors, and outputs the distance an object is from the sensor
    digitalWrite(trigPinFront, LOW); //transmit
    digitalWrite(trigPinLeft, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPinFront, HIGH); //recieve
    digitalWrite(trigPinLeft, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinFront, LOW); //transmit
    digitalWrite(trigPinLeft, LOW);
    durationFront = pulseIn(echoPinFront, HIGH); //pulse time
    durationLeft = pulseIn(echoPinLeft, HIGH);
    distanceFront = (durationFront / 2) / 29.1; //formula used to calculate distance 
    distanceLeft = (durationLeft / 2) / 29.1;

}

/*

Sensor logic checks 2 sensors, front sensor and left sensor. If activity is not detected in one sensor it checks the left sensor
If left sensor does not detect activity we assume we move right. Requires a still environment and no more than one
object moving for logic to work. Limitation of using 2 ultrasonic sensors.

Sensors have a minimum distance and a maximum distance to detect
*/
void sensorLogic() { //checks whether or not an object is seen within the preconfigured distances, and returns either true or false
    if (distanceFront >= maxdistance || distanceFront <= mindistance) {
        objectFront = false;
    } else {
        objectFront = true;
    }
    delay(500);
    if (distanceLeft >= maxdistance || distanceLeft <= mindistance) {
        objectLeft = false;
    } else {
        objectLeft = true;
    }
}

/*
Depending on detection of movement, motor logic takes care of directional movement towards detection direction.
*/
void motorLogic() {
    if (objectFront == true) { // if an object is seen up front move forward
        digitalWrite(wheelLf, HIGH); //make left wheel go forward
        digitalWrite(wheelRf, HIGH); //make right wheel go forward
        lcd.print("Following Target");
        x = 0;
    } else {
        lcd.print("Searching For Target");
    }
    if (objectLeft == true && objectFront == false) { // if an object is no longer seen up front and is seen left turn left
        digitalWrite(wheelRf, LOW); //make right wheel stop going forward
        digitalWrite(wheelLf, HIGH); //make left wheel go forward
        digitalWrite(wheelRb, HIGH); //make right wheel go backward
        delay(1); //
        digitalWrite(wheelLf, LOW); //make left wheel stop going forward
        digitalWrite(wheelRb, LOW); //make right wheel stop going backward
        x = 0;
    }
    if (objectLeft == false && objectLeft == false) { // if neither sensors see anything the object must be to the right so turn right
        digitalWrite(wheelLf, LOW); //make left wheel stop going forward
        digitalWrite(wheelRf, HIGH); //make right wheel go forward
        digitalWrite(wheelLb, HIGH); //make left wheel go forward
        delay(1); //
        digitalWrite(wheelRf, LOW); //make right wheel stop going forward
        digitalWrite(wheelLb, LOW); //make left wheel stop going backward
        x = 0;
    }
    if (distanceFront <= mindistance) { // in the scenario the user walks backwards the robot must also move backwards so it does not get trampled on
        digitalWrite(wheelRb, HIGH); //make right wheel go backward
        digitalWrite(wheelLb, HIGH); //make left wheel go forward
        x = 0;
    }

    if (objectLeft == true && objectFront == true) { 
		/*
		Because the robot does not have a stop button, 
		it will stop if both sensors detect something for 10 seconds straight
		Therefore you can stop the robot by placing 2 hands over the sensor
        */
		if (x = 0) {
            emergencyStopTime = time + 10000;
            x++;
        }
        if (time > emergencyStopTime) {
            delay(99999999);
        }


    }
}
