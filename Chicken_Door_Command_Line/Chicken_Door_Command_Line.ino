#include <DS3231.h> // RTC libary
#include <Wire.h> // I2c libary
#include <EEPROM.h> // EEPROM libary

DS3231 ds3231_rtc; // Clock object

int i_userInput; // Stores user input from serial

bool b_hour12; // Not used in 24hr mode, used in 12hr mode when getting the time (true if in 12hr mode / false if in 24hr mode)
bool b_hourPm; // Not used in 24hr mode, used in 12hr mode when getting the time (true if pm / false if am)
bool b_centuryBit; // Gets toggled when the year transions from 1999 to 2000 (I think)

bool b_autoPrintTime = false; // Toggle for printing the current date and time to the console at a 1hz rate

// 1d array makes filling EEPROM easy, this array and the writeEeprom function are only needed once and then can be deleted or commented out
const int a_initialEepromSettings[58] = {60, 60, 4, 5, 255, 255, 255, 255, 255, 255, 6, 30, 19, 0, 6, 30, 19, 0, 6, 30, 19, 30, 6, 30, 20, 30, 6, 0, 21, 0, 5, 30, 21, 30, 5, 45, 21, 30, 6, 15, 21, 0, 6, 30, 20, 45, 6, 30, 19, 30, 6, 30, 19, 30, 6, 30, 19, 0};

void writeEeprom() {
	for (int i = 0; i <= 57; i++)
	{
		EEPROM.write(i, a_initialEepromSettings[i]); // Fill EEPROM with contents of 1d array
	}
}

int a_monthOpenCloseTimes[13][4]; // 2d array makes checking if its open / close time easy

void waitForUserInput() {
	while (Serial.available() == 0) {} // Waits for user input
}

void clearUserInput() {
	while (Serial.read() >= 0); // Clears user input from serial buffer
}

int getUserInput() {
	waitForUserInput();
	i_userInput = Serial.parseInt(); // Gets user input
	clearUserInput();
}

void printMenu() {
	Serial.println(F("Settings"));
	Serial.println(F("	(1). Set RTC Time"));
	Serial.println(F("	(2). Set Open / Close Times"));
	Serial.println(F("	(3). Set RTC Date"));
	Serial.println(F("	(4). Set Door Open / Close Duration"));
	Serial.println(F("	(5). Open Door"));
	Serial.println(F("	(6). Close Door"));

	Serial.println(F("Inspection"));
	Serial.println(F("	(7). Get RTC Time / Date"));
	Serial.println(F("	(8). Get Door Open / Close Times"));
	Serial.println(F("	(9). Get Door Open / Close Duration"));

	Serial.println(F("Debug"));
	Serial.println(F("	(11). Get EEPROM"));
	Serial.println(F("	(22). Get 2D Array Elements"));
	Serial.println(F("	(33). Toggle Auto Print Current Time and Date"));
	Serial.println(F("	(44). Set EEPROM (CAUTION)"));
}

bool errorCheckUserInput(int i_valueToSet, int i_lowLimit, int i_highLimit) {
	return (i_valueToSet >= i_lowLimit && i_valueToSet <= i_highLimit);
}

void fillArrayFromEeprom() {
	// Fill index 0 with -1 so the array index corresponds to the month number
	a_monthOpenCloseTimes[0][0] = -1;
	a_monthOpenCloseTimes[0][1] = -1;
	a_monthOpenCloseTimes[0][2] = -1;
	a_monthOpenCloseTimes[0][3] = -1;

	int i_startIndex = 10; // Start storing open / close times at address 10 in EEPROM

	for (int i = 1; i <= 12; i++) // Fill 2d array with contents from EEPROM
	{
		for (int j = 0; j <= 3; j++) // Open hour is index 0, open minute is 1, close hour is 2, and close minute is 3
		{
			a_monthOpenCloseTimes[i][j] = EEPROM.read(i_startIndex);
			i_startIndex++;
		}
	}
}

void setDoorOpenCloseTime(int i_month, bool b_open, int i_hour, int i_minute) {

	int i_eepromAddress1;
	int i_eepromAddress2;

	if(!b_open) //Set open time
	{
		i_eepromAddress1 = ((i_month-1)*4)+10; // Formula to convert array position to EEPROM address, see GitHub documentation 
		i_eepromAddress2 = ((i_month-1)*4)+11;
	}
	else // Set close time
	{
		i_eepromAddress1 = ((i_month-1)*4)+12;
		i_eepromAddress2 = ((i_month-1)*4)+13;
	}

	EEPROM.write(i_eepromAddress1, i_hour);
	EEPROM.write(i_eepromAddress2, i_minute);

	fillArrayFromEeprom();
}

void commandDoor(bool b_dir) {
	// EEPROM read returns an 8bit int which has a mx value of 255, but need door open time to be larger as it needs to be in ms for delay function
	int32_t i_doorOpenTime;
	int i_doorPin; // Pin on the arduino the motor driver is connected too, storing in EEPROM in case door direction needs to be reversed
	if(b_dir) // If true, open door
	{
		Serial.println(F("Door opening..."));
		i_doorOpenTime = EEPROM.read(0); // Door open time is stored in EEPROM address 0
		i_doorPin = EEPROM.read(2); // Door open pin is stored in EEPROM address 2
	}
	else // If false, close door
	{
		i_doorOpenTime = EEPROM.read(1); // Door open time is stored in EEPROM address 1
		i_doorPin = EEPROM.read(3); // Door open pin is stored in EEPROM address 3
		Serial.println(F("Door closing ..."));
	}
	
	i_doorOpenTime = i_doorOpenTime * 1000; // Convert s to ms

	digitalWrite(i_doorPin, HIGH); // Command motor driver to move door
	delay(i_doorOpenTime); // Keep commanding for open / close duration
	digitalWrite(i_doorPin, LOW); // Stop commanding motor driver to move door

	if(b_dir)
	{
		Serial.println(F("Door opened"));
	}
	else
	{
		Serial.println(F("Door closed"));
	}

	Serial.println(F("Send anything to bring up menu"));
}

//Menu Option 1
void setRtcTime() {
	Serial.println(F("----------------------------------------------------------------------"));
	
	Serial.println(F("	Enter hours (24 hour format)"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 0, 23))
	{
		ds3231_rtc.setHour(i_userInput); // Set hours to input
	}
	else
	{
		Serial.println(F("	Invalid hours entered"));
	}

	Serial.println(F("	Enter minutes"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 0, 59))
	{
		ds3231_rtc.setMinute(i_userInput); // Set minutes to input
	}
	else
	{
		Serial.println(F("	Invalid minutes entered"));
	}

	Serial.println(F("	Enter seconds"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 0, 59))
	{
		ds3231_rtc.setSecond(i_userInput); // Set seconds to input
	}
	else
	{
		Serial.println(F("	Invalid seconds entered"));
	}

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 2
void setDoorOpenCloseTimes() {
	Serial.println(F("----------------------------------------------------------------------"));

	int i_monthNumber;
	bool i_direction;
	int i_hour;
	int i_minute;

	bool dataValid = true;

	Serial.println(F("	Enter month number (1-12)"));
	getUserInput();
	if(dataValid && errorCheckUserInput(i_userInput, 1, 12))
	{
		i_monthNumber = i_userInput;
	}
	else
	{
		dataValid = false;
		Serial.println(F("	Invalid month number"));
	}

	Serial.println(F("	(0). Set open time"));
	Serial.println(F("	(1). Set close time"));
	getUserInput();
	if(dataValid && errorCheckUserInput(i_userInput, 0, 1))
	{
		i_direction = i_userInput;
	}
	else
	{
		dataValid = false;
		Serial.println(F("		Invalid option"));
	}

	Serial.println(F("  Enter hour"));
	getUserInput();
	if(dataValid && errorCheckUserInput(i_userInput, 0, 23))
	{
		i_hour = i_userInput;
	}
	else
	{
		dataValid = false;
		Serial.println(F("		Invalid hours"));
	}
		
	Serial.println(F("  Enter minute"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 0, 59))
	{
		i_minute = i_userInput;
	}
	else
	{
		dataValid = false;
		Serial.println(F("		Invalid minutes"));
	}

	if(dataValid)
	{
		setDoorOpenCloseTime(i_monthNumber, i_direction, i_hour, i_minute);
		Serial.println(F("  Time Set"));
	}
	else
	{
		Serial.println(F("  Time NOT Set"));
	}
	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 3
void setRtcDate() {
	Serial.println(F("----------------------------------------------------------------------"));

	Serial.println(F("	Enter Month"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 1, 12))
	{
		ds3231_rtc.setMonth(i_userInput); // Set months to input
	}
	else
	{
		Serial.println(F("	Invalid month entered"));
	}
	
	Serial.println("	Enter Day");
	getUserInput();
	if(errorCheckUserInput(i_userInput, 1, 31))
	{
		ds3231_rtc.setDate(i_userInput); // Set days to input
	}
	else
	{
		Serial.println(F("	Invalid day entered"));
	}

	Serial.println(F("	Enter Year (Last 2 digits)"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 1, 99))
	{
		ds3231_rtc.setYear(i_userInput); // Set years to input
	}
	else
	{
		Serial.println(F("	Invalid year entered"));
	}

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 4
void setDoorOpenCloseDuration() {	
	Serial.println(F("----------------------------------------------------------------------"));

	Serial.println(F("	(0). Set Open Duration"));
	Serial.println(F("	(1). Set Close Duration"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 0, 1))
	{
		int i_memoryAddressToSet = i_userInput;
		if(i_userInput) //1
		{
			Serial.println(F("		Enter close door time (seconds)"));
		}
		else //0
		{
			Serial.println(F("		Enter open door time (seconds)"));
		}

		getUserInput();

		if(errorCheckUserInput(i_userInput, 1, 60))
		{
			EEPROM.write(i_memoryAddressToSet, i_userInput); // Set duration to input
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

//Menu Option 7
void getRtcTimeDate() {
	Serial.println(F("----------------------------------------------------------------------"));
	
	Serial.print(F("	Second: "));
	Serial.println(ds3231_rtc.getSecond());
	Serial.print(F("	Minute: "));
	Serial.println(ds3231_rtc.getMinute());
	Serial.print(F("	Hour: "));
	Serial.println(ds3231_rtc.getHour(b_hour12, b_hourPm));
	
	Serial.print(F("	Day: "));
	Serial.println(ds3231_rtc.getDate());
	Serial.print(F("	Month: "));
	Serial.println(ds3231_rtc.getMonth(b_centuryBit));
	Serial.print(F("	Year: "));
	Serial.println(ds3231_rtc.getYear());

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 8
void getCurrentOpenCloseTime() {
	int i_month;
	int i_currentMonth = ds3231_rtc.getMonth(b_centuryBit);

	Serial.println(F("----------------------------------------------------------------------"));

	Serial.println(F("	Enter Month (1-12) or 13 to see current month"));
	getUserInput();
	if(errorCheckUserInput(i_userInput, 1, 13))
	{
		if(i_userInput == 13)
		{
				i_month = i_currentMonth;
				Serial.print(F("	Current Month: "));
				Serial.println(i_month);
		}
		else
		{
			i_month = i_userInput;
			Serial.print(F("	Month: "));
			Serial.println(i_month);
		}

		Serial.print(F("	Open Time: "));
		Serial.print(EEPROM.read(((i_month-1)*4)+10));
		Serial.print(F(":"));
		Serial.println(EEPROM.read(((i_month-1)*4)+11));

		Serial.print(F("	Close Time: "));
		Serial.print(EEPROM.read(((i_month-1)*4)+12));
		Serial.print(F(":"));
		Serial.println(EEPROM.read(((i_month-1)*4)+13));
	}
	else
	{
		Serial.println(F("	Invalid month entered"));
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

//Menu Option 11
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
//Menu Option 22
void getArrayElements() {
	Serial.println(F("----------------------------------------------------------------------"));

	for (int i = 0; i <= 12; i++)
	{
		for (int j = 0; j <= 3; j++)
		{
			Serial.print(F("	"));
			Serial.print(i);
			Serial.print(F(": "));
			Serial.println(a_monthOpenCloseTimes[i][j]);
		}
	}
	
	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 33
void toggleAutoPrint() {
	b_autoPrintTime = (!b_autoPrintTime);

	Serial.println(F("Send anything to bring up menu"));
}
//Menu Option 44
void setEepromData() {
	int i_memoryAddress;
	int i_data;
	
	Serial.println(F("----------------------------------------------------------------------"));
	
	Serial.println(F("	Enter memory address"));
	getUserInput();
	i_memoryAddress = i_userInput;

	Serial.println(F("	Enter data"));
	getUserInput();
	i_data = i_userInput;

	Serial.print(F("	Do you want to write data: ")); // Promt user confirmation
	Serial.print(i_data);
	Serial.print(F(" to memory address: "));
	Serial.println(i_memoryAddress);

	Serial.println(F("	1 (Yes) / 0 (No)"));
	getUserInput();

	if(errorCheckUserInput(i_userInput, 0, 1))
	{
		if(i_userInput)
		{
			EEPROM.write(i_memoryAddress, i_data); // Write input to EEPROM
			fillArrayFromEeprom();
		}
	}
	else
	{
		Serial.println(F("	Invalid option entered"));
	}

	Serial.println(F("Send anything to bring up menu"));
}

void setup() {
	//writeEeprom(); // Comment out after running once
	
	Serial.begin(115200); // Start the serial port

	fillArrayFromEeprom(); // Take the open / close times out of eeprom and add to array used in checking if its time to open or close

	Wire.begin(); // Start the I2C interface

	ds3231_rtc.setClockMode(false);  // Set RTC to 24h format

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
		
		printMenu();
		getUserInput();

		switch (i_userInput)
		{
			case 1:
				setRtcTime();
				break;
			case 2:
				setDoorOpenCloseTimes();
				break;
			case 3:
				setRtcDate();
				break;
			case 4:
				setDoorOpenCloseDuration();
				break;
			case 5:
				commandDoor(true); // Open Door
				break;
			case 6:
				commandDoor(false); // Close Door
				break;
			case 7:
				getRtcTimeDate();
				break;
			case 8:
				getCurrentOpenCloseTime();
				break;
			case 9:
				getDoorOpenCloseDuration();
				break;
			case 11:
				getEepromData();
				break;
			case 22:
				getArrayElements();
				break;
			case 33:
				toggleAutoPrint();
				break;
			case 44:
				setEepromData();
				break;
			default:
				Serial.println(F("Invalid Command"));
				Serial.println(F("Send anything to bring up menu"));
				break;
		}
	}

	currentHour = ds3231_rtc.getHour(b_hour12, b_hourPm);
	currentMinute = ds3231_rtc.getMinute();
	currentSecond = ds3231_rtc.getSecond();

	currentMonth = ds3231_rtc.getMonth(b_centuryBit);

  if(currentHour == a_monthOpenCloseTimes[currentMonth][0] && currentMinute == a_monthOpenCloseTimes[currentMonth][1] && currentSecond < EEPROM.read(0)) // Open
	{
		commandDoor(true);
  }
	
  if(currentHour == a_monthOpenCloseTimes[currentMonth][2] && currentMinute == a_monthOpenCloseTimes[currentMonth][3] && currentSecond < EEPROM.read(1)) // Close
	{
		commandDoor(false);
  }

	if(b_autoPrintTime)
	{
		getRtcTimeDate(); // Print Current Time and Date
	}
      
  delay(1000);
}
