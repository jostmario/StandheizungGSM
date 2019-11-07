/*
 Name:		StandheizungGSM.ino
 Created:	06.11.2019 06:33:29
 Author:	jost Mario
 https://www.geeetech.com/wiki/index.php/Arduino_GPRS_Shield
*/

#include <TinyGPS++.h>
TinyGPSPlus gps;

#define Sim900_Serial Serial2
#define GpsNeo6_Serial Serial3

static const uint32_t SerialBaud = 115200;              // Baudrate Seraiel  für PC Serial Baud
static const uint32_t SerialBaudGSM = 19200;             // Baudrate Sim900 Modul
static const uint32_t SerialBaudGPS = 9600;
bool simm900_isINIT = false;
bool Neo6mGPS_isInit = false;
bool started = false;

// Pinbelegungen
const int Relay = 10;                   // Pin für Rellays Standheizung an         
const int switchs = 7;                  // Pin für Stanheizung Starttaster
const int esppin = 6;                   // Heizung Start
const int EspPinRueck = 5;              // Rückmeldung an ESPe
const int HeizungLed = 8;               // Led Standheizung Aktiv
const int sim900PowerPin = 7;            // Pin zum Einschalten des Sim900 Moduls
String serialInputBuffer;


void InitSim900();
void sim900_PowerOn();

// the setup function runs once when you press reset or power the boardd
void setup()
{
	//Init der Debug serial.
	Serial.begin(SerialBaud);

	//Init der GPS serial.
	GpsNeo6_Serial.begin(SerialBaudGPS);

	//Init der SIM900 serial.
	Sim900_Serial.begin(SerialBaudGSM);
	//AT an SIm900 Modul um Autobaudrate einzustellen
	Serial.println("+++ Send AT for Autobaud  +++");
	while (Sim900_Serial.read() >= 0);  // Empfangsbuffer leeren
	
	Sim900_Serial.println("AT");
	delay(1000); 

	if (Sim900_Serial.available())
	{
		// Lese alles bis zum Zeichen \n
		serialInputBuffer = "";
		serialInputBuffer = Sim900_Serial.readStringUntil('\n');
		Serial.println("Eingangspuffer: " + serialInputBuffer.substring(0, 2));

		if (serialInputBuffer.substring(0, 2) == "AT")
		{
			Serial.println("GSM 900 already on");

		}
		else
		{
			Serial.println("Error: " + serialInputBuffer);
		}
	}
	else
	{
		Serial.println("SIM 900 is off");
		sim900_PowerOn();
		Serial.println("Wait for GSM init.......");
		delay(5000);
		
	}

}

// the loop function runs over and over again until power down or reset
void loop() 
{
	// 10000 ms warten bis Modul eingebucht hat dann Initialisieren.
	if ((millis() > 10000) && (simm900_isINIT == false))
	{
		simm900_isINIT == true;
		Serial.println("+++ begin sim900 initialisierung +++");
		InitSim900();
		// InitGPSModul();
	}
			

	if (Sim900_Serial.available())
	{
		// Sendet Daten bei z.b. Anruf oder SMS
		Serial.write(Sim900_Serial.read());
	}

	
	if (Serial.available())
	{
		Sim900_Serial.write(Serial.read());
	}


	// if (GpsNeo6_Serial.available())              // GPS Sensor sendet Daten im Secundentakt raus
	// Serial.println(GpsNeo6_Serial.read());



	//delay(300);
	while (GpsNeo6_Serial.available() > 0)
	{
		gps.encode(GpsNeo6_Serial.read());
	}

	//smartDelay(200);

	////printDateTime(gps.date, gps.time);
	////Serial.println(gps.date.value()); // Raw date in DDMMYY format (u32)
	//Serial.println(gps.date.year()); // Year (2000+) (u16)
	//Serial.println(gps.date.month()); // Month (1-12) (u8)
	//Serial.println(gps.date.day()); // Day (1-31) (u8)
	//Serial.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
	Serial.println(gps.time.hour()); // Hour (0-23) (u8)
	Serial.println(gps.time.minute()); // Minute (0-59) (u8)
	////Serial.println(gps.time.second()); // Second (0-59) (u8)
	//Serial.println("#############################");

	

}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
	unsigned long start = millis();
	do
	{
		while (GpsNeo6_Serial.available())
			gps.encode(GpsNeo6_Serial.read());
	} while (millis() - start < ms);
}




void InitSim900()
{
	simm900_isINIT = false;
			
	// Serial.println("+++ begin sim900 initialisierung +++");
	delay(20);
	Serial.println("+++ Sende AT +++");
	Sim900_Serial.println("AT");
	delay(900);
	SerialAbfrage();
	Serial.println("+++ Signal Qualitaet +++ abfragen");
	Sim900_Serial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
	delay(740);
	SerialAbfrage();
	Serial.println("+++ Sim Karte abfragen +++");
	Sim900_Serial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
	delay(940);
	SerialAbfrage();
	Serial.println("+++ Check Network +++");
	Sim900_Serial.println("AT+CREG?"); //Check whether it has registered in the network
	delay(940);
	SerialAbfrage();
	Serial.println("+++ set CLIP +++");
	Sim900_Serial.println("AT+CLIP=1"); //rufnummer anzeige aktivieren CLIP
	delay(940);
	SerialAbfrage();
	Sim900_Serial.println("AT+CNMI=2,2,0,0,0");
	delay(940);
	SerialAbfrage();
	Serial.println("+++ set SMS mode to text +++");
	Sim900_Serial.println("AT+CMGF=1"); // set SMS mode to text
	delay(940);
	SerialAbfrage();
	Serial.println("+++ sms speicher leeren +++");
	Sim900_Serial.println("AT+CMGD=1,4"); // delete all SMS
	delay(940);
	SerialAbfrage();
	Serial.println("+++ sim900 fertig initialisiert +++");
	simm900_isINIT = true;
}




void sim900_PowerOn()
{
	Serial.println("+++ Sim900 wird eingeschaltet! ... +++");
	// Einschaltprozedur für Sim900 Modul
	pinMode(sim900PowerPin, OUTPUT);
	digitalWrite(sim900PowerPin, LOW);
	delay(1000);
	digitalWrite(sim900PowerPin, HIGH);
	delay(2000);
	digitalWrite(sim900PowerPin, LOW);
	delay(3000);                          // Warten bis Modul hochgefahren
}






String SerialAbfrage()
{

	while (Sim900_Serial.available())                     // Sendet Daten bei z.b. Anruf oder SMS
	Serial.write(Sim900_Serial.read());
	return Sim900_Serial.readString();

}

static void printFloat(float val, bool valid, int len, int prec)
{
	if (!valid)
	{
		while (len-- > 1)
			Serial.print('*');
		Serial.print(' ');
	}
	else
	{
		Serial.print(val, prec);
		int vi = abs((int)val);
		int flen = prec + (val < 0.0 ? 2 : 1); // . and -
		flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
		for (int i = flen; i < len; ++i)
			Serial.print(' ');
	}
	smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
	char sz[32] = "*****************";
	if (valid)
		sprintf(sz, "%ld", val);
	sz[len] = 0;
	for (int i = strlen(sz); i < len; ++i)
		sz[i] = ' ';
	if (len > 0)
		sz[len - 1] = ' ';
	Serial.print(sz);
	smartDelay(0);
}

static void printDateTime(TinyGPSDate& d, TinyGPSTime& t)
{
	if (!d.isValid())
	{
		Serial.print(F("********** "));
	}
	else
	{
		char sz[32];
		sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
		Serial.print(sz);
	}

	if (!t.isValid())
	{
		Serial.print(F("******** "));
	}
	else
	{
		char sz[32];
		sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
		Serial.print(sz);
	}

	printInt(d.age(), d.isValid(), 5);
	smartDelay(0);
}

static void printStr(const char* str, int len)
{
	int slen = strlen(str);
	for (int i = 0; i < len; ++i)
		Serial.print(i < slen ? str[i] : ' ');
	smartDelay(0);
}

