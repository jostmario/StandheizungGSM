/*
 Name:		StandheizungGSM.ino
 Created:	06.11.2019 06:33:29
 Author:	jost Mario
*/


#define Sim900_Serial Serial1
#define GpsNeo6_Serial Serial2


static const uint32_t SerialBaud = 19200;                        // Baudrate für PC Serial Baud
static const uint32_t SerialBaudGSM = 9600;                           // Baudrate Sim900 Modul
static const uint32_t SerialBaudGPS = 9600;
bool simm900_INIT = false;
bool Neo6mGPS_Init = false;


bool InitSim900();

// the setup function runs once when you press reset or power the boardd
void setup() 
{
	Serial.begin(SerialBaud);
	Sim900_Serial.begin(SerialBaudGSM);
    GpsNeo6_Serial.begin(SerialBaudGPS);
}

// the loop function runs over and over again until power down or reset
void loop() 
{
  
	if ((millis() > 10000) and (simm900_INIT == false))
	{
		Serial.println(millis());
		InitSim900();
		// InitGPSModul();
	}

}






bool InitSim900()
{
	simm900_INIT = false;
	Serial.println("+++ Begin Setup +++");
	delay(120);
	Sim900_Serial.println("AT");
	delay(140);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}
	Sim900_Serial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
	delay(140);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
	delay(140);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}


	Sim900_Serial.println("AT+CREG?"); //Check whether it has registered in the network
	delay(130);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CLIP=1"); //rufnummer anzeige aktivieren CLIP
	delay(130);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CNMI=2,2,0,0,0");
	delay(140);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}


	Sim900_Serial.println("AT+CMGF=1"); // set SMS mode to text
	delay(130);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Sim900_Serial.println("AT+CMGD=1,4"); // delete all SMS
	Serial.println("+++ All SMS geloescht +++");
	delay(130);
	while (Sim900_Serial.available())
	{
		Serial.write(Sim900_Serial.read());//Forward what Software Serial received to Serial Port
	}

	Serial.println("+++ Ende Setup +++");
	return NULL;
}