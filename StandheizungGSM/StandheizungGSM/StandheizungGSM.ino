/*
 Name:		StandheizungGSM.ino
 Created:	06.11.2019 06:33:29
 Author:	jost Mario
*/

#define Sim900_Serial Serial1
#define GpsNeo6_Serial Serial2

static const uint32_t SerialBaud = 115200;              // Baudrate für PC Serial Baud
static const uint32_t SerialBaudGSM = 19200;             // Baudrate Sim900 Modul
static const uint32_t SerialBaudGPS = 9600;
bool simm900_isINIT = false;
bool Neo6mGPS_isInit = false;

// Pinbelegungen
const int Relay = 10;                   // Pin für Relays Standheizung an
const int switchs = 7;                  // Pin für Stanheizung Starttaster
const int esppin = 6;                   // Heizung Start
const int EspPinRueck = 5;              // Rückmeldung an ESP
const int HeizungLed = 8;               // Led Standheizung Aktiv


void InitSim900();

// the setup function runs once when you press reset or power the boardd
void setup() 
{
	Serial.begin(SerialBaud);
	Sim900_Serial.begin(SerialBaudGSM);
    GpsNeo6_Serial.begin(SerialBaudGPS);
	Serial.println("booting.....");
	// Sim Modul 10 Sec Zeit geben
	delay(15000);
	// AT an SIm900 Modul um Autobaudrate einzustellen
	Serial.println("+++ sende AT fuer Autobaud +++");
	while (Sim900_Serial.read() >= 0);  // Empfangsbuffer leeren
	Sim900_Serial.println("AT");
	delay(300);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());    //Forward what Software Serial received to Serial Port
	}
	delay(300);
	Serial.println("wait for GSM init......");
}



// the loop function runs over and over again until power down or reset
void loop() 
{
	
	// 10000 ms warten bis Modul eingebucht hat dann Initialisieren.
	if ((millis() > 10000) && (simm900_isINIT == false))
	{
		Serial.println("loop");
		Serial.println(millis());
		InitSim900();
		// InitGPSModul();
	}

	if (Sim900_Serial.available())
		Serial.write(Sim900_Serial.read());
	if (Serial.available())
		Sim900_Serial.write(Serial.read());






}






void InitSim900()
{
	simm900_isINIT = false;
	Serial.println("+++ begin sim900 initialisierung +++");
	delay(3000);
	
	
	Sim900_Serial.println("AT");
	delay(200);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}
	Serial.println("+++ Signal Qualitaet +++ abfragen");
	Sim900_Serial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}


	Sim900_Serial.println("AT+CREG?"); //Check whether it has registered in the network
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CLIP=1"); //rufnummer anzeige aktivieren CLIP
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CNMI=2,2,0,0,0");
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}


	Serial.println("set SMS mode to text");
	Sim900_Serial.println("AT+CMGF=1"); // set SMS mode to text
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}
	Serial.println("sms speicher leeren");
	Sim900_Serial.println("AT+CMGD=1,4"); // delete all SMS
	Serial.println("+++ sms speicher leer +++");
	delay(240);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Serial.println("+++ sim900 initialisiert +++");
	

	simm900_isINIT = true;
}