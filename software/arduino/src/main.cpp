#include <SPI.h>
#include <Ethernet.h>

// ------------------- Serial -------------------
long baudRate = 9600; // Baudrate für beide Seriellen Kommunikationen

// ------------------- IO - Setup -------------------
#define W5500_MOSI 16          // SPI Pins for W5500
#define W5500_MISO 14          // SPI Pins for W5500
#define W5500_CLK 15           // SPI Pins for W5500
#define W5500_CS 8             // SPI Pins for W5500
#define W5500_RST 9            // SPI Pins for W5500
#define Battery_Voltage_Pin A0 // Spannungsmessung
#define Network_LED 4          // Grün: Netzwerk / Selbsttest OK
#define Battery_LED 3          // Rot: Batteriespannung niedrig
#define Buzzer 2               // Piep-Ton bei Fehler oder Warnung

void Pinmode()
{
  pinMode(W5500_CS, OUTPUT);
  pinMode(W5500_RST, OUTPUT);
  pinMode(Battery_Voltage_Pin, INPUT);
  pinMode(Network_LED, OUTPUT);
  pinMode(Battery_LED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
}

// ------------------- Netzwerk -------------------
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 159);
IPAddress server(192, 168, 0, 151);
int port = 7000;

String token = "shutdown"; // Shutdown
String testToken = "test"; // Selbsttest

EthernetClient client;

// ------------------- Batterie-Messung -------------------
const float R1 = 340000.0;
const float R2 = 100000.0;
const float Vref = 5.0;

// Baterie:     Max Voltage           14,2-14,6V (Kann nicht erreicht werden BMS?)
// Laderegler:	Ladeschluss           13,3V (~80%)
// Laderegler:	Widereinschltspannung 13,2V
const float VoltageWarning = 12.9;
const float VoltageShutdown = 12.8;
// Laderegler:	Unterspannungsschutz	12,4V
// Baterie:	Cut-Off Voltage	10V

float BatteryVoltage = 0;
int BatteryState = 2; // 0=Shutdown 1=Vorwarnung 2=OK

unsigned long intervalBatteryMeasure = 1000;    // Intervall für Analogeinlesen
unsigned long previousMillisBatteryMeasure = 0; // Intervallzwischenspeicher für Analogeinlesen

// ------------------- Networktest -------------------
const int maxRetries = 3;
int retries = 0;
int NetworkState = 1; // 0=Fehler 1=OK

unsigned long intervalNetworktest = 10000;   // Intervall für Networktest
unsigned long previousMillisNetworktest = 0; // Intervallzwischenspeicher für Networktest

// ------------------- ShutdownRequest -------------------
unsigned long intervalShutdownRequest = 5000;    // Intervall für ShutdownRequest
unsigned long previousMillisShutdownRequest = 0; // Intervallzwischenspeicher für ShutdownRequest

// ------------------- BatteryMeasure -------------------
void BatteryMeasure()
{
  float vAdc = analogRead(Battery_Voltage_Pin) * Vref / 1023.0;

  BatteryVoltage = vAdc * (R1 + R2) / R2;
  Serial.print("Batteriespannung: ");
  Serial.println(BatteryVoltage);

  BatteryState = 2;
  digitalWrite(Battery_LED, LOW);

  if (BatteryVoltage < VoltageWarning)
  {
    BatteryState = 1;
    digitalWrite(Battery_LED, HIGH);
  }

  if (BatteryVoltage < VoltageShutdown)
  {
    BatteryState = 0;
  }

  Serial.print("BatteryState: ");
  Serial.println(BatteryState);
}

// ------------------- HTTP GET Request + Antwort prüfen -------------------
bool sendRequest(String tkn, bool isTest)
{
  if (!client.connect(server, port))
  {
    Serial.println("Fehler: Verbindung zum NAS fehlgeschlagen");
    return false;
  }

  client.print("GET /shutdown_token?token=" + tkn + " HTTP/1.1\r\n");
  client.print("Host: " + String(server[0]) + "." + String(server[1]) + "." +
               String(server[2]) + "." + String(server[3]) + "\r\n");
  client.print("Connection: close\r\n\r\n");

  unsigned long timeout = millis() + 3000; // 3 Sekunden Timeout
  String response = "";
  while (client.connected() && millis() < timeout)
  {
    while (client.available())
    {
      char c = client.read();
      response += c;
    }
  }
  client.stop();

  if (isTest)
  {
    // Nur beim Test prüfen, ob "Token accepted" enthalten ist
    return response.indexOf("Token accepted") >= 0;
  }
  return true; // ShutdownRequest: nur auf connect angewiesen
}

// ------------------- ShutdownRequest -------------------
void sendShutdownRequest()
{
  if (!sendRequest(token, false))
  {
    Serial.println("ShutdownRequest: Fehler konnte nicht gesendet werden!");
  }
  else
  {
    Serial.println("ShutdownRequest: Erfolgreich gesendet!");
  }
}

// ------------------- Selbsttest-Funktion -------------------
void Networktest()
{
  if (sendRequest(testToken, true))
  {
    retries = 0;
    NetworkState = 1;
    Serial.println("Netzwerktest: OK");
    digitalWrite(Network_LED, LOW);
  }
  else
  {
    retries = retries + 1;
    Serial.print("Netzwerktest: fehlgeschlagen ");
    Serial.print(retries);
    Serial.println("x!");
    digitalWrite(Network_LED, HIGH);
  }

  if (retries >= maxRetries)
  {
    Serial.println("Netzwerktest: Error");
    NetworkState = 0;
  }
}

void setup()
{

  Pinmode();                    // Pis Initialisieren
  digitalWrite(W5500_RST, LOW); // W5500 Resetten
  delay(50);
  digitalWrite(W5500_RST, HIGH);
  delay(50);

  Serial.begin(baudRate);

  // W5500 mit CS Pin initialisieren
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, ip);

  delay(5000);
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  digitalWrite(Network_LED, LOW);
  digitalWrite(Battery_LED, LOW);
  digitalWrite(Buzzer, LOW);
}

void loop()
{
  // ------------------- BatteryMeasure -------------------
  if (millis() - previousMillisBatteryMeasure > intervalBatteryMeasure)
  {
    previousMillisBatteryMeasure = millis();
    BatteryMeasure();
  }

  // ------------------- BatteryMeasure -------------------
  if (millis() - previousMillisNetworktest > intervalNetworktest)
  {
    previousMillisNetworktest = millis();
    Networktest();
  }

  // ------------------- ShutdownRequest -------------------
  if (millis() - previousMillisShutdownRequest > intervalShutdownRequest)
  {
    if (BatteryState == 0)
    {
      Serial.println("ShutdownRequest: Spannung unter Shutdown-Schwelle! Request senden...");
      sendShutdownRequest();
      previousMillisShutdownRequest = millis();
    }
  }

  // ------------------- SetBuzzer -------------------
  if (BatteryState < 2 || NetworkState == 0)
  {
    tone(Buzzer, 2000); // 2 kHz Warnton
  }
  else
  {
    noTone(Buzzer);
  }
}