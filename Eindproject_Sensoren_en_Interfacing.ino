#define BLYNK_TEMPLATE_ID "....."
#define BLYNK_TEMPLATE_NAME ".........."
#define BLYNK_PRINT Serial

// Add these libraries to use the DHT sensor and to use the Blynk App
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Define the pins for the sensors and actuators
#define DHT_PIN 4          // The DHT11 measures temperature and humidity
#define ldrPin 35          // The LDR measures the light strength of the environment so that the light comes on when it is too dark
#define bvPin 34           // The soil moisture sensor measures soil moisture so the pump waters at the right time
#define VentilatorTemp 18  // This fan blows air into the greenhouse so regulates the temperature
#define VentilatorHum 21   // This fan pulls air from the greenhouse so regulates humidity
#define Licht 23           // The lamp turns on when it is too dark
#define Verwarming 26      // The heating comes on when it is too cold
#define Waterpomp 25       // The water pump comes on when the ground is too dry

// Put your auth code of your Blynk project, WiFi name and WiFi password here
char auth[] = "......................";
char ssid[] = "............";
char pass[] = "...............";

// Define the DHT sensor
DHT dht(DHT_PIN, DHT11);

// These variables reflect the measured temperature, humidity, light and soil moisture
float gemetenTemp = 0;
float gemetenHum = 0;
float gemetenLicht = 0;
float gemetenBodemv = 0;

// These variables are used to set the parameters for the actuators.
float minTemp = 0;    // This variable ensures that when the temperature goes below this value, that the heater turns on
float maxTemp = 0;    // This variable ensures that when the temperature goes above this value, that the fan for the temperature turns on
float minLicht = 0;   // This variable ensures that when the light percentage goes below this value, that then the lamp turns on
float maxHum = 0;     // This variable ensures that when the humidity percentage goes above this value, that then the humidity fan turns on
float minBodemv = 0;  // This variable ensures that when the soil moisture percentage goes below this value, that then the water pump turns on

// These variables are for what controls and parameters you want to use
bool bediening = true;
bool parameters = true;

// This function allows you to switch between types of controls in your Blynk App (based on the sensors or manually)
BLYNK_WRITE(V0) {
  bediening = !bediening;
}

// This function allows you to switch between parameters (default or custom) in your Blynk App
BLYNK_WRITE(V20) {
  parameters = !parameters;
}

// You can use these functions to modify the parameters in your Blynk App if the 'parameters' variable is 'false'
BLYNK_WRITE(V21) {
  if (!parameters) {
    minTemp = param.asFloat();
  }
}
BLYNK_WRITE(V22) {
  if (!parameters) {
    maxTemp = param.asFloat();
  }
}
BLYNK_WRITE(V23) {
  if (!parameters) {
    maxHum = param.asFloat();
  }
}
BLYNK_WRITE(V24) {
  if (!parameters) {
    minLicht = param.asFloat();
  }
}
BLYNK_WRITE(V25) {
  if (!parameters) {
    minBodemv = param.asFloat();
  }
}

// In the setup you start the serial communication, connect to your Blynk project, start the DHT11 sensor and also define which pins are input and output
void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "...", 8081);  // Make sure your port is also set to 8081 and that you enter your own domain on the dots
  dht.begin();
  pinMode(VentilatorTemp, OUTPUT);
  pinMode(VentilatorHum, OUTPUT);
  pinMode(Licht, OUTPUT);
  pinMode(Verwarming, OUTPUT);
  pinMode(Waterpomp, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(bvPin, INPUT);
}

void loop() {
  // At the beginning of the loop function you start your Blynk and the sensor values are always read
  Blynk.run();
  sensorLezing();
  // Then if your variable 'bediening' is 'true' then the 'sensorVerwerking' function is called
  if (bediening) {
    sensorVerwerking();
  }
  // If the variable 'bediening' is 'false' then the 'sensorVerwerking' function is skipped and the 'actuatoren' function is started
  actuatoren();
  delay(3000);
}

// This function reads the values of the sensors and sends them to the Blynk app
void sensorLezing() {
  gemetenTemp = dht.readTemperature();
  gemetenHum = dht.readHumidity();
  gemetenLicht = map(analogRead(ldrPin), 0, 4096, 100, 0); //Map the analogue values so that you obtain a percentage
  gemetenBodemv = map(analogRead(bvPin), 0, 4096, 100, 0); //Map the analogue values so that you obtain a percentage

  Blynk.virtualWrite(V10, gemetenTemp);
  Blynk.virtualWrite(V11, gemetenHum);
  Blynk.virtualWrite(V12, gemetenLicht);
  Blynk.virtualWrite(V13, gemetenBodemv);
}

// This function processes the values of the sensors so that each actuator turns on or off at the right time
void sensorVerwerking() {
  // If the variable 'parameters' is 'true', then these parameters are used. So these are default. You can, of course, change these to your own desire.
  if (parameters) {
    minTemp = 18;
    maxTemp = 24;
    minLicht = 50;
    maxHum = 60;
    minBodemv = 30;
  }
  // If the variable 'parameters' is 'false', then the default values are not set and so you can set them yourself in your Blynk app.
  if (gemetenTemp >= maxTemp) {
    digitalWrite(VentilatorTemp, HIGH);
  } else if (gemetenTemp < maxTemp && gemetenTemp > minTemp) {
    digitalWrite(VentilatorTemp, LOW);
    digitalWrite(Verwarming, LOW);
  } else {
    digitalWrite(Verwarming, HIGH);
  }
  if (gemetenHum >= maxHum) {
    digitalWrite(VentilatorHum, HIGH);
  } else {
    digitalWrite(VentilatorHum, LOW);
  }
  if (gemetenLicht > minLicht) {
    digitalWrite(Licht, LOW);
  } else {
    digitalWrite(Licht, HIGH);
  }
  if (gemetenBodemv > minBodemv) {
    digitalWrite(Waterpomp, LOW);

  } else {
    digitalWrite(Waterpomp, HIGH);
  }
}

// This feature displays the actual state of each actuator in the Blynk app regardless of the values of the sensors
void actuatoren() {
  if (digitalRead(VentilatorTemp) == HIGH) {
    Blynk.virtualWrite(V6, "Ventil.Tem = aan");
  } else if (digitalRead(VentilatorTemp) == LOW && digitalRead(Verwarming) == LOW) {
    Blynk.virtualWrite(V6, "Vent.& Verw.=uit");
  } else {
    Blynk.virtualWrite(V6, "Verwarming = aan");
  }
  if (digitalRead(VentilatorHum) == HIGH) {
    Blynk.virtualWrite(V7, "Ventil.Hum = aan");
  } else {
    Blynk.virtualWrite(V7, "Ventil.Hum = uit");
  }
  if (digitalRead(Licht) == HIGH) {
    Blynk.virtualWrite(V5, "Licht = aan");
  } else {
    Blynk.virtualWrite(V5, "Licht = uit");
  }
  if (digitalRead(Waterpomp) == HIGH) {
    Blynk.virtualWrite(V8, "Pomp = aan");
  } else {
    Blynk.virtualWrite(V8, "Pomp = uit");
  }
}
