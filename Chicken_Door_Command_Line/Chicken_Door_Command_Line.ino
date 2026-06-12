#include <DS3231.h> // RTC libary
#include <Wire.h> // I2c libary
#include <EEPROM.h> // EEPROM libary

DS3231 myRTC; // Clock object

int incomingInt; // Stores user input from serial

bool h12; // Not used in 24hr mode, used in 12hr mode when getting the time (true if in 12hr mode / false if in 24hr mode)
bool hPM; // Not used in 24hr mode, used in 12hr mode when getting the time (true if pm / false if am)
bool CenturyBit; // Gets toggled when the year transions from 1999 to 2000 (I think)

bool printTime = false; // Toggle for printing the current date and time to the console at a 1hz rate

//1d array makes filling EEPROM easy, this array and the writeEEPROM function are only needed once and then can be deleted or commented out
const int sch1d[58] = {60, 60, 4, 5, 255, 255, 255, 255, 255, 255, 6, 30, 19, 0, 6, 30, 19, 0, 6, 30, 19, 30, 6, 30, 20, 30, 6, 0, 21, 0, 5, 30, 21, 30, 5, 45, 21, 30, 6, 15, 21, 0, 6, 30, 20, 45, 6, 30, 19, 30, 6, 30, 19, 30, 6, 30, 19, 0};

void writeEEPROM() {
	for (int i = 0; i <= 57; i++)
	{
		EEPROM.write(i, sch1d[i]); // Fill EEPROM with contents of 1d array
	}
}

//2d array makes checking if its open / close time easy (probably replace with a struct)
int sch[13][4];

void waitForUserInput() {
	while (Serial.available() == 0) {} // Wait for user input
}

void clearUserInput() {
	while (Serial.read() >= 0); // Clear user input
}

int getUserInput() {
	waitForUserInput(); // Wait for user input
	incomingInt = Serial.parseInt(); // Get user input
	clearUserInput(); // Clear input
}

void printMenu() {
	// Print menu options
	Serial.println(F("(1). Set RTC Time"));
	Serial.println(F("(2). Set EEPROM"));
	Serial.println(F("(3). Toggle Auto Print Current Time and Date"));
	Serial.println(F("(4). Set RTC Date"));
	Serial.println(F("(5). Set Door Open / Close Duration"));
	Serial.println(F("(6). Get RTC Time / Date"));
	Serial.println(F("(7). Get Elements In Open / Close Time Array"));
	Serial.println(F("(8). Get EEPROM"));
	Serial.println(F("(9). Get Door Open / Close Duration"));
	Serial.println(F("(88). Open Door"));
	Serial.println(F("(99). Close Door"));
}

bool errorCheckUserInput(int valueToSet, int lowLimit, int highLimit) {
	return (valueToSet >= lowLimit && valueToSet <= highLimit);
}

void fillArrayFromEeprom() {
	// Fill index 0 with -1 so the array index corresponds to the month number
	sch[0][0] = -1;
	sch[0][1] = -1;
	sch[0][2] = -1;
	sch[0][3] = -1;

	int startIndex = 10; // Start storing open / close times at address 10 in EEPROM

	for (int i = 1; i <= 12; i++) // Fill 2d array with contents from EEPROM
	{
		for (int j = 0; j <= 3; j++)
		{
			sch[i][j] = EEPROM.read(startIndex);
			startIndex++;
		}
	}
}

void commandDoor(bool dir) {
	int32_t doorOpenTime;
	int doorPin;
	if(dir) // If true, open door
	{
		Serial.println(F("Door opening..."));
		doorOpenTime = EEPROM.read(0); // Door open time is stored in EEPROM address 0
		doorPin = EEPROM.read(2); // Door open time is stored in EEPROM address 2
	}
	else // If false, close door
	{
		doorOpenTime = EEPROM.read(1); // Door open time is stored in EEPROM address 1
		doorPin = EEPROM.read(3); // Door open time is stored in EEPROM address 3
		Serial.println(F("Door closing ..."));
	}
	
	doorOpenTime = doorOpenTime * 1000; // Convert ms to s

	digitalWrite(doorPin, HIGH); // Command motor driver to move door
	delay(doorOpenTime); // Keep commanding for open / close duration
	digitalWrite(doorPin, LOW);

	if(dir)
	{
		Serial.println(F("Door opened"));
	}
	else
	{
		Serial.println(F("Door closed"));
	}
}

//Menu Option 1
void setRtcTime() {
	Serial.println(F("----------------------------------------------------------------------"));
	
	Serial.println(F("	Enter hours (24 hour format)"));
	getUserInput();
	if(errorCheckUserInput(incomingInt, 0, 23)) // Error check hours
	{
		myRTC.setHour(incomingInt); // Set hours to input
	}
	else
	{
		Serial.println(F("	Invalid hours entered"));
	}

	Serial.println(F("	Enter minutes"));
	getUserInput();
	if(errorCheckUserInput(incomingInt, 0, 59)) // Error check minutes
	{
		myRTC.setMinute(incomingInt); // Set minutes to input
	}
	else
	{
		Serial.println(F("	Invalid minutes entered"));
	}

	Serial.println(F("	Enter seconds"));
	getUserInput();
	if(errorCheckUserInput(incomingInt, 0, 59)) // Error check seconds
	{
		myRTC.setSecond(incomingInt); // Set seconds to input
	}
	else
	{
		Serial.println(F("	Invalid seconds entered"));
	}

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 2
void setEepromData() {
	int memoryAddress;
	int data;
	
	Serial.println(F("----------------------------------------------------------------------"));
	
	Serial.println(F("	Enter memory address"));
	getUserInput();
	memoryAddress = incomingInt;

	Serial.println(F("	Enter data"));
	getUserInput();
	data = incomingInt;

	Serial.print(F("	Do you want to write data: ")); // Promt user confirmation
	Serial.print(data);
	Serial.print(F(" to memory address: "));
	Serial.println(memoryAddress);

	Serial.println(F("	1 (Yes) / 0 (No)"));
	getUserInput();

	if(errorCheckUserInput(incomingInt, 0, 1)) // Get user input
	{
		if(incomingInt)
		{
			EEPROM.write(memoryAddress, data);
			fillArrayFromEeprom();
		}
	}
	else
	{
		Serial.println(F("	Invalid option entered"));
	}

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 3
void toggleAutoPrint() {
	printTime = (!printTime);

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 4
void setRtcDate() {
	Serial.println(F("----------------------------------------------------------------------"));

	Serial.println(F("	Enter Month")); // Prompt user input
	getUserInput();
	if(errorCheckUserInput(incomingInt, 1, 12)) // Error check months
	{
		myRTC.setMonth(incomingInt); // Set months to input
	}
	else
	{
		Serial.println(F("	Invalid month entered"));
	}
	
	Serial.println("	Enter Day"); // Prompt user input
	getUserInput();
	if(errorCheckUserInput(incomingInt, 1, 31)) // Error check days
	{
		myRTC.setDate(incomingInt); // Set days to input
	}
	else
	{
		Serial.println(F("	Invalid day entered"));
	}

	Serial.println(F("	Enter Year (Last 2 digits)")); // Prompt user input
	getUserInput();
	if(errorCheckUserInput(incomingInt, 1, 99)) // Error check years
	{
		myRTC.setYear(incomingInt); // Set years to input
	}
	else
	{
		Serial.println(F("	Invalid year entered"));
	}

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 5
void setDoorOpenCloseDuration() {	
	Serial.println(F("----------------------------------------------------------------------"));

	Serial.println(F("	(0). Set Open Duration")); // Prompt user input
	Serial.println(F("	(1). Set Close Duration")); // Prompt user input
	getUserInput();
	if(errorCheckUserInput(incomingInt, 0, 1))
	{
		int memoryAddressToSet = incomingInt;
		if(incomingInt) //1
		{
			Serial.println(F("		Enter close door time (seconds)")); // Prompt user
		}
		else //0
		{
			Serial.println(F("		Enter open door time (seconds)")); // Prompt user
		}

		getUserInput();

		if(errorCheckUserInput(incomingInt, 1, 60))
		{
			EEPROM.write(memoryAddressToSet, incomingInt); // Set user input
		}
		else
		{
			Serial.println(F("		Invalid duration"));
		}
	}
	else
	{
		Serial.println(F("	Invalid option entered"));
	}
	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 6
void getRtcTimeDate() {
	Serial.println(F("----------------------------------------------------------------------"));
	
	Serial.print(F("	Second: "));
	Serial.println(myRTC.getSecond());
	Serial.print(F("	Minute: "));
	Serial.println(myRTC.getMinute());
	Serial.print(F("	Hour: "));
	Serial.println(myRTC.getHour(h12, hPM));
	
	Serial.print(F("	Day: "));
	Serial.println(myRTC.getDate());
	Serial.print(F("	Month: "));
	Serial.println(myRTC.getMonth(CenturyBit));
	Serial.print(F("	Year: "));
	Serial.println(myRTC.getYear());

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 7
void getArrayElements() {
	Serial.println(F("----------------------------------------------------------------------"));

	for (int i = 0; i <= 12; i++)
	{
		for (int j = 0; j <= 3; j++)
		{
			Serial.print(F("	"));
			Serial.print(i);
			Serial.print(F(": "));
			Serial.println(sch[i][j]);
		}
	}
	
	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 8
void getEepromData() {
	Serial.println(F("----------------------------------------------------------------------"));
	
	while (Serial.read() >= 0);
	for (int i = 0; i <= 57; i++)
	{
		Serial.print(F("	"));
		Serial.print(i);
		Serial.print(F(": "));
		Serial.println(EEPROM.read(i));
	}

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 9
void getDoorOpenCloseDuration() {
	Serial.println(F("----------------------------------------------------------------------"));
	
	while (Serial.read() >= 0);
	Serial.print(F("Open Time: "));
	Serial.println(EEPROM.read(0));
	Serial.print(F("Close Time: "));
	Serial.println(EEPROM.read(1));
	
	Serial.println(F("Send anything to bring up menu"));
}

void setup() {
	//writeEEPROM(); // Comment out after running once
	
	Serial.begin(115200); // Start the serial port

	fillArrayFromEeprom(); // Take the times out of eeprom and add to array used in checking if its time to open or close

	Wire.begin(); // Start the I2C interface

	myRTC.setClockMode(false);  // Set RTC to 24h format

	pinMode(4, OUTPUT); // Set pin mode for open
	digitalWrite(4, LOW); // Set default to off
	pinMode(5, OUTPUT); // Set pin mode for close
	digitalWrite(5, LOW); // Set default to off
}

void loop() {
	int currentHour;
	int currentMinute;
	int currentSecond;
	int currentMonth;
	
	if (Serial.available() > 0) // Listen for input to bring up menu
	{
		clearUserInput(); // Clear input used to bring up menu
		
		printMenu(); // Print Options
		getUserInput();

		switch (incomingInt) // Get user command
		{
			case 1:
				setRtcTime(); // Call function for action
				break;
			case 2:
				setEepromData(); // Call function for action
				break;
			case 3:
				toggleAutoPrint(); // Call function for action
				break;
			case 4:
				setRtcDate(); // Call function for action
				break;
			case 5:
				setDoorOpenCloseDuration(); // Call function for action
				break;
			case 6:
				getRtcTimeDate(); // Call function for action
				break;
			case 7:
				getArrayElements(); // Call function for action
				break;
			case 8:
				getEepromData(); // Call function for action
				break;
			case 9:
				getDoorOpenCloseDuration(); // Call function for action
				break;
			case 88:
				commandDoor(true); // Call function for action
				break;
			case 99:
				commandDoor(false); // Call function for action
				break;
			default:
				Serial.println(F("Invalid Command"));
				Serial.println(F("Send anything to bring up menu"));
				break;
		}
	}

	currentHour = myRTC.getHour(h12, hPM);
	currentMinute = myRTC.getMinute();
	currentSecond = myRTC.getSecond();

	currentMonth = myRTC.getMonth(CenturyBit);

  if(currentHour == sch[currentMonth][0] && currentMinute == sch[currentMonth][1] && currentSecond < EEPROM.read(0)) // Open
	{
		commandDoor(true);
  }
	
  if(currentHour == sch[currentMonth][2] && currentMinute == sch[currentMonth][3] && currentSecond < EEPROM.read(1)) // Close
	{
		commandDoor(false);
  }

	if(printTime)
	{
		getRtcTimeDate(); // Print Current Time and Date
	}
      
  delay(1000);
}
