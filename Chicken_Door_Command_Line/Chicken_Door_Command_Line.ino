#include <DS3231.h>
#include <Wire.h>
#include <EEPROM.h>

DS3231 myRTC;

int incomingByte;

bool h12;
bool hPM;
bool CenturyBit;

int currentHour;
int currentMinute;
int currentSecond;

int currentMonth;

bool printTime = false;

int sch[13][4];

void setRtcDate()
{
	while (Serial.read() >= 0);
	Serial.println("Enter Month");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	myRTC.setMonth(incomingByte);

	while (Serial.read() >= 0);
	Serial.println("Enter Day");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	myRTC.setDate(incomingByte);
	
	while (Serial.read() >= 0);
	Serial.println("Enter Year (last 2 digits)");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	myRTC.setYear(incomingByte);

	while (Serial.read() >= 0);
	Serial.println("Success");
}

void setRtcTime()
{
	while (Serial.read() >= 0);
	Serial.println("Enter hours (24 hour format)");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	myRTC.setHour(incomingByte);

	while (Serial.read() >= 0);
	Serial.println("Enter minutes");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	myRTC.setMinute(incomingByte);
	
	while (Serial.read() >= 0);
	Serial.println("Enter seconds");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	myRTC.setSecond(incomingByte);

	while (Serial.read() >= 0);
	Serial.println("Success");
}

void getRtcTimeDate()
{
	while (Serial.read() >= 0);
	Serial.print("Second: ");
	Serial.println(myRTC.getSecond());
	Serial.print("Minute: ");
	Serial.println(myRTC.getMinute());
	Serial.print("Hour: ");
	Serial.println(myRTC.getHour(h12, hPM));
	
	Serial.print("Day: ");
	Serial.println(myRTC.getDate());
	Serial.print("Month: ");
	Serial.println(myRTC.getMonth(CenturyBit));
	Serial.print("Year: ");
	Serial.println(myRTC.getYear());
	Serial.println();
}

void setTimeDatePrint()
{
	while (Serial.read() >= 0);
	Serial.println("(0). Print Time Off");
	Serial.println("(1). Print Time ON");
	while (Serial.available() == 0) 
	{
	}
	incomingByte = Serial.parseInt();
	printTime = incomingByte;
	while (Serial.read() >= 0);
	Serial.println("Success");
}

void setDoorOpenCloseDuration()
{
	while (Serial.read() >= 0);
		
	Serial.println("(1). Set Open Duration");
	Serial.println("(2). Set Close Duration");
	
	while (Serial.available() == 0) 
	{
	}

	incomingByte = Serial.parseInt();

	switch (incomingByte)
	{
		case 1:
			while (Serial.read() >= 0);
			Serial.println("Enter open door time (seconds)");
			while (Serial.available() == 0) 
			{
			}
			incomingByte = Serial.parseInt();
			EEPROM.write(0, incomingByte);
			while (Serial.read() >= 0);
			Serial.println("Success");
			break;
		case 2:
			while (Serial.read() >= 0);
			Serial.println("Enter close door time (seconds)");
			while (Serial.available() == 0) 
			{
			}
			incomingByte = Serial.parseInt();
			EEPROM.write(1, incomingByte);
			while (Serial.read() >= 0);
			Serial.println("Success");
			break;
		default:
			Serial.println("Invalid Option");
			break;
	}
}

void getDoorOpenCloseDuration()
{
	while (Serial.read() >= 0);
	Serial.print("Open Time: ");
	Serial.println(EEPROM.read(0));
	Serial.print("Close Time: ");
	Serial.println(EEPROM.read(1));
	Serial.println("Success");
}

void fillArrayFromEeprom()
{
	sch[0][0] = -1;
	sch[0][1] = -1;
	sch[0][2] = -1;
	sch[0][3] = -1;

	int startIndex = 10;

	for (int i = 1; i <= 12; i++)
	{
		for (int j = 0; j <= 3; j++)
		{
			sch[i][j] = EEPROM.read(startIndex);
			startIndex++;
		}
	}
}

void getOpenCloseTimes()
{
	while (Serial.read() >= 0);
	for (int i = 0; i <= 57; i++)
	{
		Serial.print(i);
		Serial.print(": ");
		Serial.println(EEPROM.read(i));
	}
	Serial.println("Success");
}

void setOpenCloseTimes()
{
	int memoryAddress;
	int data;
	
	while (Serial.read() >= 0);
	Serial.println("Enter memory address");
	while (Serial.available() == 0) 
	{
	}
	memoryAddress = Serial.parseInt();
	while (Serial.read() >= 0);
	Serial.println("Enter data");
	while (Serial.available() == 0) 
	{
	}
	data = Serial.parseInt();
	while (Serial.read() >= 0);
	EEPROM.write(memoryAddress, data);
	fillArrayFromEeprom();
	Serial.println("Success");
}

void getArrayElements()
{
	while (Serial.read() >= 0);

	for (int i = 0; i <= 12; i++)
	{
		for (int j = 0; j <= 3; j++)
		{
			Serial.print(i);
			Serial.print(": ");
			Serial.println(sch[i][j]);
		}
	}
}

void doorOpen()
{
	int32_t doorOpenTime = EEPROM.read(0);
	doorOpenTime = doorOpenTime * 1000;

	digitalWrite(4,HIGH);
	Serial.println("Opening");
	delay(doorOpenTime); //Open Duration
	digitalWrite(4,LOW);
	Serial.println("Done Opening Door");
}

void doorClose()
{
	int32_t doorCloseTime = EEPROM.read(0);
	doorCloseTime = doorCloseTime * 1000;	

	digitalWrite(5,HIGH);
	Serial.println("Closing");
	delay(doorCloseTime); //Close Duration
	digitalWrite(5,LOW);
	Serial.println("Done Closing Door");
}

void writeEEPROM()
{
	EEPROM.write(0, 60); // Write the value 60 to address 0
	EEPROM.write(1, 60); // Write the value 60 to address 1

	EEPROM.write(3, 1); //Normal operation, set to 0 to flip door open / close function

	//Jan
	EEPROM.write(10, 6);
	EEPROM.write(11, 30);
	EEPROM.write(12, 19);
	EEPROM.write(13, 0);

	//Feb
	EEPROM.write(14, 6);
	EEPROM.write(15, 30);
	EEPROM.write(16, 19);
	EEPROM.write(17, 0);

	//Mar
	EEPROM.write(18, 6);
	EEPROM.write(19, 30);
	EEPROM.write(20, 19);
	EEPROM.write(21, 30);

	//Apr
	EEPROM.write(22, 6);
	EEPROM.write(23, 30);
	EEPROM.write(24, 20);
	EEPROM.write(25, 30);

	//May
	EEPROM.write(26, 6);
	EEPROM.write(27, 0);
	EEPROM.write(28, 21);
	EEPROM.write(29, 0);

	//June
	EEPROM.write(30, 5);
	EEPROM.write(31, 30);
	EEPROM.write(32, 21);
	EEPROM.write(33, 30);

	//July
	EEPROM.write(34, 5);
	EEPROM.write(35, 45);
	EEPROM.write(36, 21);
	EEPROM.write(37, 30);

	//Aug
	EEPROM.write(38, 6);
	EEPROM.write(39, 15);
	EEPROM.write(40, 21);
	EEPROM.write(41, 0);

	//Sep
	EEPROM.write(42, 6);
	EEPROM.write(43, 30);
	EEPROM.write(44, 20);
	EEPROM.write(45, 45);

	//Oct
	EEPROM.write(46, 6);
	EEPROM.write(47, 30);
	EEPROM.write(48, 19);
	EEPROM.write(49, 30);

	//Nov
	EEPROM.write(50, 6);
	EEPROM.write(51, 30);
	EEPROM.write(51, 19);
	EEPROM.write(53, 30);

	//Dec
	EEPROM.write(54, 6);
	EEPROM.write(55, 30);
	EEPROM.write(56, 19);
	EEPROM.write(57, 0);
}

void setup() 
{
	//Comment out after running once
	//writeEEPROM();
	
	// Start the serial port
	Serial.begin(115200);

	fillArrayFromEeprom();
	
	// Start the I2C interface
	Wire.begin();

	myRTC.setClockMode(false);  // set to 24h

	pinMode(4, OUTPUT);
	digitalWrite(4, LOW);
	pinMode(5, OUTPUT);
	digitalWrite(5, LOW);
}

void loop() 
{
	//Set RTC Time, RTC Date,
	if (Serial.available() > 0) 
	{
		while (Serial.read() >= 0);
		
		Serial.println("(1). Set RTC Date");
		Serial.println("(2). Set RTC Time");
		Serial.println("(3). Set Time Date Print");
		Serial.println("(4). Get RTC Time Date");
		Serial.println("(5). Set Door Open/Close Duration");
		Serial.println("(6). Get Door Open/Close Duration");
		Serial.println("(7). Set Open Close Times");
		Serial.println("(8). Get Open Close Times");
		Serial.println("(9). Get Array Elements");

		while (Serial.available() == 0) 
		{
		}

		incomingByte = Serial.parseInt();

		switch (incomingByte)
		{
			case 1:
				setRtcDate(); 
				break;
			case 2:
				setRtcTime();
				break;
			case 3:
				setTimeDatePrint();
				break;
			case 4:
				getRtcTimeDate();
				break;
			case 5:
				setDoorOpenCloseDuration();
				break;
			case 6:
				getDoorOpenCloseDuration();
				break;
			case 7:
				setOpenCloseTimes();
				break;
			case 8:
				getOpenCloseTimes();
				break;
			case 9:
				getArrayElements();
				break;
			default:
				Serial.println("Invalid Command");
				break;
		}
	}

	currentHour = myRTC.getHour(h12, hPM);
	currentMinute = myRTC.getMinute();
	currentSecond = myRTC.getSecond();

	currentMonth = myRTC.getMonth(CenturyBit);

  //Open
  if(currentHour == sch[currentMonth][0] && currentMinute == sch[currentMonth][1] && currentSecond < EEPROM.read(0))
	{
		if(EEPROM.read(3)) //set to 1 for true, set to 0 for false
		{
			doorOpen();
		}
		else
		{
			doorClose();
		}
  }
	//Close
  if(currentHour == sch[currentMonth][2] && currentMinute == sch[currentMonth][3] && currentSecond < EEPROM.read(1))
	{
		if(EEPROM.read(3))
		{
			doorClose();
		}
		else
		{
			doorOpen();
		}
  }

	if(printTime)
	{
		//Print Current Time and Date
		getRtcTimeDate();
	}
      
  delay(1000);
}