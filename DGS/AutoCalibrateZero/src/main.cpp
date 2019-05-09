#include <Arduino.h>
#include "DGS\DGS.cpp"
#include "SoftwareSerial.h"

SoftwareSerial Serial1(10, 11);  // !!! RX, TX Must be on 3.3 volt communication, or using level shifters to get to 3.3V UART!!!
DGS mySensor(&Serial1);

bool const setupSensor = false;
String const setupBarcode = "061318011357 110406 O3 1807 -44.53";
bool calibratedZero = false;

void setup() {
	Serial.begin(9600);
	Serial1.begin(9600);
	Serial.flush();
	Serial1.flush();
	Serial.println("Init...");
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, 1);	
	delay(10000);  // Important!!! Need 10s for get EEPROM data
	
	mySensor.DEBUG = false;
	mySensor.getEEPROM();
	mySensor.DEBUG = true;
	delay(2000);

	// Checked compatibility with firmware 15SEP17 and 25SEP17
	Serial.println("Firmware: " + mySensor.getFW());
	Serial.println("Barcode: " + mySensor.getBarcode());

	if (setupSensor) {
		Serial.println("Setup...");
		// Enter your barcode string. Returns 1/0
		mySensor.setBC(setupBarcode);

		// Loads LMP values into variable returns 1/0
		if (mySensor.getLMP()) {
			Serial.print("LMP91000 Values loaded into array");
			if (mySensor.setLMP(mySensor.LMP[0], mySensor.LMP[1], mySensor.LMP[2])) Serial.println("Finished Setting New Values For LMP");  //Sets LMP variables into register returns 1/0
		}

		// Set T offset in deg C returns 1/0. Podemos subir/bajar la temperatura. Hay que pasar el valor a subir ej "1.2" o a bajar "-2.2"
		// if (mySensor.setToff(0.0)) Serial.println("Finished Setting T Offset");
	}

	Serial.println("");
}

void blinkLed() {
	digitalWrite(LED_BUILTIN, 1);
	delay(10);
	digitalWrite(LED_BUILTIN, 0);
}

void loop() {
	mySensor.getData('\r');
	
	if (!calibratedZero)
		blinkLed();  // blink while not calibrated

	Serial.print(mySensor.getGasName() + " " + String(mySensor.getConc()) + " ppb");  // default is 'p' for temperature compensated ppb, any other cahracter for raw counts
	Serial.print(" | T " + String(mySensor.getTemp('C')));         // 'F' or 'C' for units of temperature, default is 'C'
	Serial.print(" | H " + String(mySensor.getRh()));
	Serial.println(" | " + String(mySensor.getTimeRunning()));

	if ((mySensor.getTimeRunning() == "0d 08:00:00") || 
		(mySensor.getTimeRunning() == "0d 08:00:01")) {
		// Auto Zero al time
		if (mySensor.zero()) Serial.println("Finished Setting Zero");  //zeros returns 1/0
		calibratedZero = true;
	}

	if (Serial.available()) {
		char U = Serial.read();

		if (U == 'Z')
			if (mySensor.zero()) Serial.println("Finished Setting Zero");  //zeros returns 1/0

		if ((U == 'r') || (U == 'R'))
			mySensor.resetModule();

		if (U == 'f')
			Serial.println("My firmware: " + mySensor.getFW());

		if (U == 'e') {
			mySensor.DEBUG = true;
			mySensor.getEEPROM();
		}

		if (U == 'S') {
			// No siempre funciona bien, algunas veces tarda hasta 5min en ejecutarse
			Serial.print("Enter span value in PPM (ex 10.2): ");
			delay(200);

			String cadena = "";
			while (true) {
				while (!Serial.available()) {
				}
				char val = Serial.read();
				Serial.print(val);

				if (val == '\n')
					break;

				cadena = cadena + val;
			}
			mySensor.setXSpan(cadena.toFloat());
		}
	}
}
