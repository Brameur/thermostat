#define BLYNK_TEMPLATE_ID   "user9"
#define BLYNK_TEMPLATE_NAME "user9@server.wyns.it"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Blynk instellingen
char auth[] = "8Wle5tD5B__VxYCnc7kVadJXSRUZWZEm";
char ssid[] = "embed";
char pass[] = "weareincontrol";
char server[] = "server.wyns.it";
int port = 8081; // Poortnummer voor de Blynk-server

// DHT11 instellingen
#define DHTPIN 2       // Pin waarop de DHT11 is aangesloten
#define DHTTYPE DHT11  // Type DHT-sensor

DHT dht(DHTPIN, DHTTYPE);

// Potentiometer instellingen
#define POT_PIN 34     // Pin waarop de potentiometer is aangesloten
#define POT_MAX_VALUE 4095.0  // Maximale waarde van de potentiometer (12-bit resolutie)
#define TEMPERATURE_RANGE 30.0  // Gewenste temperatuur range
#define POT_TOLERANCE_PERCENT 3.0 // Tolerantie van de potentiometer in procenten

// Virtuele pinnen voor Blynk
#define V1 1 // Virtuele pin voor gemeten temperatuur
#define V2 2 // Virtuele pin voor gewenste temperatuur
#define V3 3 // Virtuele pin voor slider (gewenste temperatuur instellen)
#define V4 4 // Virtuele pin voor LED-waarde

#define LAMP_PIN 12 // Pin voor de lamp

float gemetenTemperatuur = 0.0;
float gewensteTemperatuur = 0.0; // Start bij 0 graden
float laatstAangepasteWaarde = gewensteTemperatuur; // Nieuwe variabele toevoegen
int laatstGelezenPotValue = 0; // Variabele om de laatst gelezen potentiometerwaarde op te slaan
int LedValueOn;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, server, port);

  dht.begin();

  pinMode(POT_PIN, INPUT);
  pinMode(LAMP_PIN, OUTPUT);

  // Configuratie van virtuele pinnen
  Blynk.virtualWrite(V1, gemetenTemperatuur);
  Blynk.virtualWrite(V2, gewensteTemperatuur);

  // Blynk Slider-widget
  Blynk.virtualWrite(V3, gewensteTemperatuur);
  Blynk.virtualWrite(V3, 0, TEMPERATURE_RANGE); // Stel de minimale en maximale waarde van de slider in
}

void loop() {
  Blynk.run();
  meetTemperatuur();
}

void meetTemperatuur() {
  gemetenTemperatuur = dht.readTemperature();

  int potValue = analogRead(POT_PIN);

  // Lees potentiometer bij elke verandering
  if (abs(potValue - laatstGelezenPotValue) > (POT_MAX_VALUE * POT_TOLERANCE_PERCENT / 100.0)) {
    float potGewensteTemperatuur = map(potValue, 0, POT_MAX_VALUE, 0, TEMPERATURE_RANGE);
    gewensteTemperatuur = max(potGewensteTemperatuur, laatstAangepasteWaarde);
    laatstGelezenPotValue = potValue; // Update de laatst gelezen waarde
  }

  Serial.print("Gemeten temperatuur: ");
  Serial.print(gemetenTemperatuur);
  Serial.print(" °C, Gewenste temperatuur: ");
  Serial.print(gewensteTemperatuur);
  Serial.println(" °C");

  Blynk.virtualWrite(V1, gemetenTemperatuur);
  Blynk.virtualWrite(V2, gewensteTemperatuur);

  // Zet de lamp aan als de gewenste temperatuur hoger is dan de gemeten temperatuur
  if (gewensteTemperatuur > gemetenTemperatuur) {
    LedValueOn = 1023;
    digitalWrite(LAMP_PIN, HIGH);
  } else {
    LedValueOn = 0;
    digitalWrite(LAMP_PIN, LOW);
  }

  Blynk.virtualWrite(V4, LedValueOn);
  delay(1000);
}

BLYNK_WRITE(V3) {
  gewensteTemperatuur = param.asFloat();
  Serial.print("Nieuwe gewenste temperatuur: ");
  Serial.println(gewensteTemperatuur);

  // Update de variabele met de laatst aangepaste waarde
  laatstAangepasteWaarde = gewensteTemperatuur;
}
