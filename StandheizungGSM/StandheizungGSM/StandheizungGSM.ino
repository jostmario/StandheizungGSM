/*
 Name:		StandheizungGSM.ino
 Created:	06.11.2019 06:33:29
 Author:	jost Mario
*/


static const uint32_t SerialBaudDebug = 19200;                        // Baudrate für PC Serial Baud
static const uint32_t SerialBaudGSM = 9600;                           // Baudrate Sim900 Modul
static const uint32_t SerialBaudGPS = 9600;
bool simm900_INIT = false;
bool Neo6mGPS_Init = false;


bool InitSim900();

// the setup function runs once when you press reset or power the boardd
void setup() 
{

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
	simm900_INIT = true;
	Serial.println("+++ Begin Setup +++");
	delay(120);
	Serial1.println("AT");
	delay(140);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}
	Serial1.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
	delay(140);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}

	Serial1.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
	delay(140);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}


	Serial1.println("AT+CREG?"); //Check whether it has registered in the network
	delay(130);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}

	Serial1.println("AT+CLIP=1"); //rufnummer anzeige aktivieren CLIP
	delay(130);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}

	Serial1.println("AT+CNMI=2,2,0,0,0");
	delay(140);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}


	Serial1.println("AT+CMGF=1"); // set SMS mode to text
	delay(130);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}

	Serial1.println("AT+CMGD=1,4"); // delete all SMS
	Serial.println("+++ All SMS geloescht +++");
	delay(130);
	while (Serial1.available())
	{
		Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
	}

	Serial.println("+++ Ende Setup +++");
	return NULL;
}