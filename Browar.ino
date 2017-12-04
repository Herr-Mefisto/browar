#include <OneWire.h>
#include <DS18B20.h>
#include <LiquidCrystal.h> 

//LED
#define HEATER_LED_PIN 6
#define MIX_LED_PIN 13
#define ERROR_LED_PIN 7

//BUTTONS
#define PLUS_BUTTON_PIN 9
#define MINUS_BUTTON_PIN 8
#define MIX_BUTTON_PIN 10
#define CHANGE_BUTTON_PIN 2

//TEMP SENSOR
#define ONEWIRE_PIN 1

//MIXER
#define MIX_PIN 0

//HEATERS
#define HEATER1_PIN 3
#define HEATER2_PIN 4

#define INVALID_TEMPERATURE -100
#define ROUNDF(f, c) (((float)((int)((f) * (c))) / (c)))

#define LOOP_DELAY 200
#define HISTERESIS 0.5f

byte temperatureSensorAddress[8] = { 0x28, 0xFF, 0x32, 0x8f, 0xC0, 0x16, 0x4, 0x78 };
byte temperatureCounter = 0;
float lastTemperature = INVALID_TEMPERATURE;
bool isTemperatureSet = true;
unsigned char requiredTemperature = 20;
bool mixingEnabled = false;
unsigned char mixerSpeed = 0;

OneWire oneWire(ONEWIRE_PIN);
DS18B20 sensors(&oneWire);
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

void printTemperature(float temperature) {

	lcd.setCursor(0, 0); //Ustawienie kursora w odpowiednim momencie
	lcd.print("BROWAR ORZESKI");

	lcd.setCursor(11, 1);
	float roundedTemp = ROUNDF(temperature, 10);
	lcd.print(roundedTemp);
	lcd.setCursor(15, 1);
	lcd.print("C");

	lcd.setCursor(7, 1);
	lcd.print(requiredTemperature);
	lcd.print("C");

	lcd.setCursor(0, 1);
	lcd.print(mixerSpeed);
	lcd.print("%");
}

float getCurrentTemperature() {
	if (temperatureCounter > 5 && sensors.available())
	{
		float temperature = sensors.readTemperature(temperatureSensorAddress);
		sensors.request(temperatureSensorAddress);
		temperatureCounter = 0;
		lastTemperature = temperature;
		return temperature;
	}

	temperatureCounter++;
	sensors.request(temperatureSensorAddress);

	return lastTemperature;
}

void turnOnLed(int pinNumber)
{
	digitalWrite(pinNumber, HIGH);
}

void turnOffLed(int pinNumber) {
	digitalWrite(pinNumber, LOW);
}

void turnOnMixer() {
	analogWrite(100, MIX_PIN);
}

void turnOffMixer() {
	analogWrite(0, MIX_PIN);
}

void turnOnHeater() {
	digitalWrite(HEATER_LED_PIN, HIGH);
	digitalWrite(HEATER1_PIN, HIGH);
	digitalWrite(HEATER2_PIN, HIGH);
}

void turnOffHeater() {
	digitalWrite(HEATER_LED_PIN, LOW);
	digitalWrite(HEATER1_PIN, LOW);
	digitalWrite(HEATER2_PIN, LOW);
}

void handleButtons()
{
	if (digitalRead(MIX_BUTTON_PIN) == LOW)
	{
		mixingEnabled = !mixingEnabled;
	}

	if (digitalRead(PLUS_BUTTON_PIN) == LOW)
	{
		if (isTemperatureSet == true && requiredTemperature < 110)
			requiredTemperature++;
		else if (!isTemperatureSet == true && mixerSpeed <= 95)
			mixerSpeed += 5;
	}

	if (digitalRead(MINUS_BUTTON_PIN) == LOW)
	{
		if (isTemperatureSet == true && requiredTemperature > 10)
			requiredTemperature--;
		else if (!isTemperatureSet == true && mixerSpeed >= 5)
			mixerSpeed -= 5;
	}

	if (digitalRead(CHANGE_BUTTON_PIN) == LOW)
	{
		Serial.println("CHANGE_BUTTON_PIN");
		isTemperatureSet = !isTemperatureSet;
	}
}

void handleMixer()
{
	Serial.println(isTemperatureSet);
	if (mixingEnabled == true)
	{
		turnOnLed(MIX_LED_PIN);
		turnOnMixer();
	}
	else
	{
		turnOffLed(MIX_LED_PIN);
		turnOffMixer();
	}
}

void handleHeater(float currentTemperature) {
	if (currentTemperature < requiredTemperature - HISTERESIS)
	{
		turnOnHeater();
	}
	else if (currentTemperature > requiredTemperature + HISTERESIS)
	{
		turnOffHeater();
	}
}

void setup() {
	pinMode(14, OUTPUT);
	pinMode(15, OUTPUT);
	pinMode(16, OUTPUT);
	pinMode(17, OUTPUT);
	pinMode(18, OUTPUT);
	pinMode(19, OUTPUT);
	pinMode(HEATER_LED_PIN, OUTPUT);
	pinMode(MIX_LED_PIN, OUTPUT);
	pinMode(ERROR_LED_PIN, OUTPUT);
	pinMode(PLUS_BUTTON_PIN, INPUT_PULLUP);
	pinMode(MINUS_BUTTON_PIN, INPUT_PULLUP);
	pinMode(MIX_BUTTON_PIN, INPUT_PULLUP);
	pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
	//Temperature sensor initialization
	sensors.begin();
	sensors.request(temperatureSensorAddress);
	lcd.setCursor(0, 0); //Ustawienie kursora w odpowiednim momencie
	lcd.begin(16, 2);
	lcd.print("BROWAR ORZESKI");
}

void loop()
{
	handleButtons();

	handleMixer();

	float currentTemperature = getCurrentTemperature();
	handleHeater(currentTemperature);
	printTemperature(currentTemperature);
	delay(LOOP_DELAY);
}