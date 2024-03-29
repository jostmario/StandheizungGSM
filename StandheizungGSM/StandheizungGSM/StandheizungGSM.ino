/*
 Name:		StandheizungGSM.ino
 Created:	06.11.2019 06:33:29
 Author:	jost Mario
 https://www.geeetech.com/wiki/index.php/Arduino_GPRS_Shield
*/

#include <TinyGPS++.h>
#include <Time.h>
//#include <iostream>
//#include <string>

//using namespace std;

TinyGPSPlus gps;

#define Sim900_Serial Serial1
#define GpsNeo6_Serial Serial2

bool SmsAntwort = false;                                  // Soll eine Best�tigungsmail gesendet werden (verursacht Kosten)
static const uint32_t SerialBaud = 115200;              // Baudrate Seraiel  f�r PC Serial Baud
static const uint32_t SerialBaudGSM = 19200;             // Baudrate Sim900 Modul
static const uint32_t SerialBaudGPS = 9600;
bool simm900_isINIT = false;
bool Neo6mGPS_isInit = false;
bool started = false;
unsigned long startTime;
unsigned long shutdownTime;
bool RelayOn;
String serialInputBufferGSM;
const unsigned long DEFAULTTIME = 5000;                // Standart Laufzeit der Standheizung 30 Min 1800000ms
const bool EIN = true;
const bool AUS = false;
int val = 0;
int valoff = 0;
int HeizLedState = LOW;
unsigned long previousMillis = 0;                      // LED Blink will store last time LED was updated
const long interval = 200;                             // LED Blink interval at which to blink Heizungs Led (milliseconds)
unsigned long currentMilliss = millis();               // LED Blink
String startzeitH = "HH";
String startzeitM = "MM";
String aktuelleZeitH;
String aktuelleZeitM;
unsigned long TimeDiffInMilli;
unsigned long TimeToStartInMilli;
bool isUeberlauf = false;
bool isUeberlaufRelayOff = false;
bool isStartzeitAktiv = false;

// Pinbelegungen
const int Relay = 10;                   // Pin f�r Rellays Standheizung an         
const int switchs = 7;                  // Pin f�r Stanheizung Starttaster
const int switchsoff = 9;
const int esppin = 6;                   // Heizung Start
const int EspPinRueck = 5;              // R�ckmeldung an ESPe
const int HeizungLed = 8;               // Led Standheizung Aktiv
const int sim900PowerPin = 6;            // Pin zum Einschalten des Sim900 Moduls


void InitSim900();
void sim900_PowerOn();
boolean isNumeric(String str);
unsigned long GetTimeDiff(long _aktuelleZeitH, long _aktuelleZeitM, long _startzeitH, long _startzeitM);

// the setup function runs once when you press reset or power the boardd
void setup()
{
	pinMode(switchs, INPUT_PULLUP);            // Taster als Input
	digitalWrite(switchs, HIGH);        // Intern Pullup setzen
	
	pinMode(switchsoff, INPUT_PULLUP);            // Taster als Input
	digitalWrite(switchsoff, HIGH);        // Intern Pullup setzen

	pinMode(HeizungLed, OUTPUT);      // Status LED Standheizung
	pinMode(Relay, OUTPUT);           // Relays als Output
	digitalWrite(Relay, LOW);         // Relays aus setzen
	

	//Init der Debug serial.
	Serial.begin(SerialBaud);
	Serial.println("+++ SMS Antwort deaktiviert Aktivieren mit SMS komando -->  smson  oder  smsoff  +++");

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
		serialInputBufferGSM = "";
		serialInputBufferGSM = Sim900_Serial.readStringUntil('\n');
		// Serial.println("Eingangspuffer: " + serialInputBufferGSM.substring(0, 2));
	
		

		if (serialInputBufferGSM.substring(0, 2) == "AT")
		{
			Serial.println(" +++ GSM 900 bereits an +++");
			Serial.println(" +++ Wait for GSM init....... +++");
		}
		else
		{
			Serial.println("Fehler: " + serialInputBufferGSM);
		}
	}
	else
	{
		Serial.println(" +++ SIM 900 is off +++");
		sim900_PowerOn();
		Serial.println(" +++ Wait for GSM init....... +++");
		delay(5000);

	}
	while (Sim900_Serial.read() >= 0);  // Empfangsbuffer leeren
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	
	//Serial.println(millis());

	// 10000 ms warten bis Modul eingebucht hat dann Initialisierenn.
	if ((millis() > 2000) && (simm900_isINIT == false))
	{
		
		simm900_isINIT == true;
		Serial.println(" +++ begin sim900 initialisierung +++");
		InitSim900();
		// InitGPSModul();
	}

	if (isStartzeitAktiv)
	{
		if (!RelayOn) 
		{		
			if (millis() >= TimeToStartInMilli && !isUeberlauf)
			{
				//Serial.println(millis());
				HeizungEin(EIN, DEFAULTTIME);
				isStartzeitAktiv = false;
			}

			if (millis() < 5)
			{
				isUeberlauf = false;
			}
		}
	}

	// ++++++++++++++ Per Taster standheizung einschalten Start++++++++++++++++++++++++++
	val = digitalRead(switchs);   // read the input pin
  // Serial.println(val);
	if (val == 0)
	{
		if (!RelayOn)
		{
			Serial.println(" +++ Aktiviert durch Taster +++");
			HeizLedState = HIGH;
			HeizungEin(EIN, DEFAULTTIME);
			DeleteAllSMS();      // SMS Speicher leeren
		}

	}
	// ++++++++++++++ Per Taster standheizung einschalten Ende++++++++++++++++++++++++++
	


	valoff = digitalRead(switchsoff);   // read the input pin
  // Serial.println(valoff);
	if (valoff == 0)
	{
		if (RelayOn)
		{
			Serial.println(" +++ Standheizung aus durch Taster +++");
			HeizLedState = LOW;
			HeizungEin(AUS, DEFAULTTIME);

		}
    }

	if (Sim900_Serial.available())
	{
			
		serialInputBufferGSM = Sim900_Serial.readStringUntil('\n');
		delay(10);
		serialInputBufferGSM.toLowerCase();
		Serial.println(serialInputBufferGSM);
		
		
		//  ################################### Anruf ############################
		// Ein Anruf kommt rein mit der nummer 491622742063
		// if (serialInputBufferGSM.substring(8, 12) == "CLIP:\"+491622742063\"")
		//if (serialInputBufferGSM.substring(8, 14) == "491622")
		if (serialInputBufferGSM.startsWith("+clip"))
		{
			Serial.println("+++ Registrierte Rufnummern:  +491622742063   +4974126950510   +4915903778464 +++");    
			if (serialInputBufferGSM.substring(8, 21) == "+491622742063" || "+4974126950510" || "+4915903778464" )
			{
				Serial.println(serialInputBufferGSM.substring(8, 14));
				Serial.println("Heizung Einschalten via Telefon");
				HeizungEin(EIN, DEFAULTTIME);
			}
		}
		else if (serialInputBufferGSM.startsWith("+cmt"))
		{
			aktuelleZeitH = serialInputBufferGSM.substring(35, 37);			
			aktuelleZeitM = serialInputBufferGSM.substring(38, 40);

			Serial.println("Aktuelle Stunde: " + aktuelleZeitH);
			Serial.println("Aktuelle Minute: " + aktuelleZeitM);
		}


			//######### SMS Empfang #########
			//  on    off    timer15    start0730
			//HeizungSteuernUhrzeit(bool Zustand, string time)
			// alle sms befehle in kleinschreibung umwandeln
			
		
			if (serialInputBufferGSM.substring(0, 3) == "off")
			{
				HeizungEin(AUS, 0);
				DeleteAllSMS();
				Serial.println("Off durch SMS");
				isStartzeitAktiv = false;
				isUeberlauf = false;
				isUeberlaufRelayOff = false;
			}
			else if (serialInputBufferGSM.substring(0, 2) == "on")
			{
				HeizungEin(EIN, DEFAULTTIME);
				DeleteAllSMS();
				Serial.println("On durch SMS");
			}
			else if (serialInputBufferGSM.substring(0, 5) == "timer")
			{
				HeizungEin(EIN, (serialInputBufferGSM.substring(5, 7).toInt()) * 60000);
				DeleteAllSMS();
				Serial.println("Timer durch SMS");
			}
			else if (serialInputBufferGSM.substring(0, 4) == "init")
			{
				InitSim900();
			}
			else if (serialInputBufferGSM.substring(0, 5) == "start")
			{
				startzeitH = serialInputBufferGSM.substring(5, 7);
				startzeitM = serialInputBufferGSM.substring(7, 9);
				Serial.println("Startzeit programmiert auf " + startzeitH + " " + startzeitM);
				TimeDiffInMilli = GetTimeDiff(aktuelleZeitH.toInt(), aktuelleZeitM.toInt(), startzeitH.toInt(), startzeitM.toInt());
				TimeToStartInMilli = millis() + TimeDiffInMilli;

				Serial.print("Zeitdifferenz in Millisekunden: ");
				Serial.println(TimeDiffInMilli);
				Serial.print("Zeit in Millisekunden bis Start: ");
				Serial.println(TimeToStartInMilli);

				if (millis() > TimeToStartInMilli)
				{
					isUeberlauf = true;
					Serial.println("+++ Millis wird ueberlaufen +++");
				}

				isStartzeitAktiv = true;				
			}
			else if (serialInputBufferGSM.substring(0, 5) == "smson")
			{
				SmsAntwort = true;                                  // Soll eine Best�tigungsmail gesendet werden (verursacht Kosten)
				Serial.println("+++ AntwortSMS aktiviert +++");
			}
			else if (serialInputBufferGSM.substring(0, 5) == "smsoff")
			{
				SmsAntwort = false;                                  // Soll eine Best�tigungsmail gesendet werden (verursacht Kosten)
				Serial.println("+++ AntwortSMS deaktiviert +++");
			}
	}


	
	if (Serial.available())
	{
		Sim900_Serial.write(Serial.read());
	}


	// Relays nach 30 min Abschalten
	if (RelayOn)
	{
		if (millis() > startTime + shutdownTime && !isUeberlaufRelayOff)
		{
			HeizungEin(AUS, 0);
			digitalWrite(EspPinRueck, LOW);   // Heizung an an ESP8622 melden
			DeleteAllSMS();      // SMS Speicher leeren
		}

		if (millis() < 5)
		{
			isUeberlaufRelayOff = false;
		}
	}


  // ####################  Blinkende LED f�r Standheizung Aktiv ###############################
  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval)
	{
		previousMillis = currentMillis;                                 // save the last time you blinked the LED
		if (RelayOn) 
		{
			if (HeizLedState == LOW)
			{
				HeizLedState = HIGH;
				// Serial.println("test Heizung An"); 
			}
			else
			{
				HeizLedState = LOW;
				// Serial.println("Led Heizung Aus");
			}
			digitalWrite(HeizungLed, HeizLedState);                              // set the LED with the ledState of the variable:
		}
		else digitalWrite(HeizungLed, LOW);
	}


	// if (GpsNeo6_Serial.available())              // GPS Sensor sendet Daten im Secundentakt raus
	// Serial.println(GpsNeo6_Serial.read());



	//delay(300);
	/*while (GpsNeo6_Serial.available() > 0)
	{
		gps.encode(GpsNeo6_Serial.read());
	}*/

	//smartDelay(200);

	////printDateTime(gps.date, gps.time);
	////Serial.println(gps.date.value()); // Raw date in DDMMYY format (u32)
	//Serial.println(gps.date.year()); // Year (2000+) (u16)
	//Serial.println(gps.date.month()); // Month (1-12) (u8)
	//Serial.println(gps.date.day()); // Day (1-31) (u8)
	//Serial.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
	// Serial.println(gps.time.hour()); // Hour (0-23) (u8)
	// Serial.println(gps.time.minute()); // Minute (0-59) (u8)
	////Serial.println(gps.time.second()); // Second (0-59) (u8)
	//Serial.println("#############################");

	

}

// This custom version of delay() ensures that the gps object
// is being "fed".




//static void smartDelay(unsigned long ms)
//{
//	unsigned long start = millis();
//	do
//	{
//		while (GpsNeo6_Serial.available())
//			gps.encode(GpsNeo6_Serial.read());
//	} while (millis() - start < ms);
//}




void InitSim900()
{
	simm900_isINIT = false;
			
	// Serial.println("+++ begin sim900 initialisierung +++");
	delay(20);
	Serial.println("+++ Sende AT +++");
	Sim900_Serial.println("AT");
	delay(800);
	SerialAbfrage();
	Serial.println("+++ Signal Qualitaet +++ abfragen");
	Sim900_Serial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
	delay(800);
	SerialAbfrage();
	Serial.println("+++ Sim Karte abfragen +++");
	Sim900_Serial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
	delay(800);
	SerialAbfrage();
	Serial.println("+++ Check Network +++");
	Sim900_Serial.println("AT+CREG?"); //Check whether it has registered in the network
	delay(800);
	SerialAbfrage();
	Serial.println("+++ set CLIP +++");
	Sim900_Serial.println("AT+CLIP=1"); //rufnummer anzeige aktivieren CLIP
	delay(800);
	SerialAbfrage();
	Sim900_Serial.println("AT+CNMI=2,2,0,0,0");
	delay(800);
	SerialAbfrage();
	Serial.println("+++ set SMS mode to text +++");
	Sim900_Serial.println("AT+CMGF=1"); // set SMS mode to text
	delay(800);
	SerialAbfrage();
	Serial.println("+++ sms speicher leeren +++");
	Sim900_Serial.println("AT+CMGD=1,4"); // delete all SMS
	delay(800);
	SerialAbfrage();
	Serial.println("+++ sim900 fertig initialisiert +++");
	simm900_isINIT = true;
}







void sim900_PowerOn()
{
	Serial.println("+++ Sim900 wird eingeschaltet! ... +++");
	// Einschaltprozedur f�r Sim900 Modul
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









void DeleteAllSMS()
{
	Sim900_Serial.println("AT+CMGD=1,4"); // delete all SMS
	Serial.println("Alle SMS geloescht void");
	delay(700);
	Sim900_Serial.println("AT+CMGF=1");
	delay(700);

	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

}



//
//static void printFloat(float val, bool valid, int len, int prec)
//{
//	if (!valid)
//	{
//		while (len-- > 1)
//			Serial.print('*');
//		Serial.print(' ');
//	}
//	else
//	{
//		Serial.print(val, prec);
//		int vi = abs((int)val);
//		int flen = prec + (val < 0.0 ? 2 : 1); // . and -
//		flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
//		for (int i = flen; i < len; ++i)
//			Serial.print(' ');
//	}
//	smartDelay(0);
//}
//
//static void printInt(unsigned long val, bool valid, int len)
//{
//	char sz[32] = "*****************";
//	if (valid)
//		sprintf(sz, "%ld", val);
//	sz[len] = 0;
//	for (int i = strlen(sz); i < len; ++i)
//		sz[i] = ' ';
//	if (len > 0)
//		sz[len - 1] = ' ';
//	Serial.print(sz);
//	smartDelay(0);
//}
//
//static void printDateTime(TinyGPSDate& d, TinyGPSTime& t)
//{
//	if (!d.isValid())
//	{
//		Serial.print(F("********** "));
//	}
//	else
//	{
//		char sz[32];
//		sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
//		Serial.print(sz);
//	}
//
//	if (!t.isValid())
//	{
//		Serial.print(F("******** "));
//	}
//	else
//	{
//		char sz[32];
//		sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
//		Serial.print(sz);
//	}
//
//	printInt(d.age(), d.isValid(), 5);
//	smartDelay(0);
//}
//
//static void printStr(const char* str, int len)
//{
//	int slen = strlen(str);
//	for (int i = 0; i < len; ++i)
//		Serial.print(i < slen ? str[i] : ' ');
//	smartDelay(0);
//}

boolean isNumeric(String str) {
	unsigned int stringLength = str.length();

	if (stringLength == 0) {
		return false;
	}

	boolean seenDecimal = false;

	for (unsigned int i = 0; i < stringLength; ++i) {
		if (isDigit(str.charAt(i))) {
			continue;
		}

		if (str.charAt(i) == '.') {
			if (seenDecimal) {
				return false;
			}
			seenDecimal = true;
			continue;
		}
		return false;
	}
	return true;
}

unsigned long GetTimeDiff(long _aktuelleZeitH, long _aktuelleZeitM, long _startzeitH, long _startzeitM)
{
	long resultHour;
	long resultMinute;
	unsigned long resultMilli;

	resultHour = _startzeitH - _aktuelleZeitH;
	resultMinute = _startzeitM - _aktuelleZeitM;
	
	Serial.println("+++++++++ GetTimeDiff: +++++++++");
	Serial.print("aktuelleZeitH: ");
	Serial.println(_aktuelleZeitH);
	Serial.print("aktuelleZeitM: ");
	Serial.println(_aktuelleZeitM);
	Serial.print("startzeitH: ");
	Serial.println(_startzeitH);
	Serial.print("startzeitM: ");
	Serial.println(_startzeitM);


	if (resultHour < 0)
	{
		resultHour = resultHour + 24;
	}

	if (_aktuelleZeitM > _startzeitM)
	{
		resultHour = resultHour - 1;
	}

	if (resultMinute < 0)
	{
		resultMinute = resultMinute + 60;
	}


	resultMilli = resultHour * 3600 * 1000;
	resultMilli = resultMilli + (resultMinute * 60 * 1000);

	Serial.print("resultHour: ");
	Serial.println(resultHour);
	Serial.print("resultMinute: ");
	Serial.println(resultMinute);
	Serial.print("resultMilli: ");
	Serial.println(resultMilli);

	return resultMilli;
}


void HeizungEin(bool state, unsigned long shutdownValue)
{
	if (state)
		{
		
			// Serial.println("Abschaltzeit: " + shutdownValue);
			Serial.println("Relay ON");
			digitalWrite(Relay, HIGH);
			digitalWrite(EspPinRueck, HIGH);   // Heizung an an ESP8622 melden
			RelayOn = true;
			startTime = millis();
			shutdownTime = shutdownValue;

			if (millis() > startTime + shutdownTime)
			{
				isUeberlaufRelayOff = true;
			}

			if (SmsAntwort)
			{
				smssend();
				Serial.println("+++ Sende SMS +++");
			}
			// Serial.println(startTime);
			// Serial.println(shutdownTime);
			// Serial.println(startTime + shutdownTime);
		}
		else
		{
			Serial.println("Relay OFF");
			digitalWrite(Relay, LOW);
			digitalWrite(EspPinRueck, LOW);   // Heizung an an ESP8622 melden
			RelayOn = false;
		}
}



void smssend() {
	Sim900_Serial.println("AT+CMGF=1\r");
	delay(100);
	Sim900_Serial.println("AT + CMGS = \"+491622742063\"");
	delay(100);
	Sim900_Serial.println("Standheizung ON");
	delay(100);
	Sim900_Serial.println((char)26);
	delay(100);
	Sim900_Serial.println();
	delay(5000);
}