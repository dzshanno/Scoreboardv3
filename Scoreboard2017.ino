// MAIN CODE FOR WALLINGFORD HC SCOREBOARD
// MARCH 2017 VERSION 3.0
// DAVID SHANNON

// INCLUDE REQUIRED HEADER FILES


#include <Bounce2.h>
#include <RTClib.h> //REAL TIME CLOCK FILE
#include <Adafruit_NeoPixel.h> // LED STRIP FILE
#include <Wire.h> // NOT SURE WHAT this is for
#include <arduino.h>

// PIN DEFINITIONS

#define LEDPIN      6 // data pin for LED strip
#define GSMTXPIN    9 // DATA PIN FOR TRANSMITTING MESSAGES TO GSM SIM
#define GSMRXPIN    10 // DATA PIN FOR RECEIVING MESSAGES FROM GSM SIM
#define TEMPPIN     11 // DATA PIN FOR TEMPERATURE SENSOR
#define LIGHTPIN    12 // DATA PIN FOR LIGHT SENSOR
#define HOMEBUTTONPIN     7 // PIN FOR HOME GOAL BUTTON
#define AWAYBUTTONPIN     8 // PIN FOR AWAY GOAL BUTTON
#define MODEBUTTONPIN     3// PIN FOR MODE BUTTON
#define SETBUTTONPIN     2 // PIN FOR SET BUTTON

// set up debouncers

Bounce debounceHome = Bounce();
Bounce debounceAway = Bounce();
Bounce debounceMode = Bounce();
Bounce debounceSet = Bounce();



// Define constants

#define N_LEDS 240

// Define colors to make setting the LEDs easier
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define WHITE  0xFFFFFF
#define OFF    0X000000


// define global variables

byte Brightness = 100;
long TimerStartTime = 0;
long TimerDuration = 0;
long TimerDisplayTime = 0;
long TimeNow = 0;
long Timer1 = 0;
long SwitchOnTime;
byte HomeScore = 0;
byte AwayScore = 0;
long temp = 0;


boolean ClockFlash = false;

enum  ScoreboardModes { Reset, Clock, Timer, Temp, SetClockHour, SetClockMin, SetTimerMin, SetTimerSec };
enum TimerStatuses { Unset, Set, Running, Paused, Finished};

uint32_t ClockDisplayColor = BLUE;
uint32_t TimerDisplayColor = GREEN;
uint32_t ScoreDisplayColor = RED;
uint32_t TempDisplayColor = RED;

RTC_DS1307 RTC;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, LEDPIN, NEO_GRB + NEO_KHZ800);


// Font definitions ////////////////////////////
// 7 segments set up as

//  33333
//  4   2
//  11111
//  5   7
//  66666

bool Font[12][7] = {
	{ 0,1,1,1,1,1,1 }, //value 0
	{ 0,1,0,0,0,0,1 }, //value 1
	{ 1,1,1,0,1,1,0 }, //value 2
	{ 1,1,1,0,0,1,1 }, //value 3
	{ 1,1,0,1,0,0,1 }, //value 4
	{ 1,0,1,1,0,1,1 }, //value 5
	{ 1,0,1,1,1,1,1 }, //value 6
	{ 0,1,1,0,0,0,1 }, //value 7
	{ 1,1,1,1,1,1,1 }, //value 8
	{ 1,1,1,1,0,0,1 }, //value 9
	{ 0,0,0,0,0,0,0 }, //value 10 - all off
    { 1,1,1,1,0,0,0 }  // value 11 - degree sign
};

// digit set up

// int LedSegmentMapSmall[] = { 5,5,5,4,4,5,5 };
// int LedSegmentMapLarge[] = { 7,7,7,5,5,7,7 };
// int LedSegmentMapColon[] = { 2,2 };




byte LedSegmentMap[6][7][2] = {
	{//first clock digit
		{ 0,5 }, // segment 0
		{ 5,5 }, // segment 1
		{ 10,5 }, // segment 2
		{ 15,4 },// segment 3
		{ 19,4 }, // segment 4
		{ 23,5 }, // segment 5
		{ 28,5 } // segment 6
	},

	{//second clock digit
		{ 33,5 },
		{ 38,5 },
		{ 43,5 },
		{ 48,4 },
		{ 52,4 },
		{ 56,5 },
		{ 61,5 }

	},
	{//third clock digit
		{ 74,5 },
		{ 79,5},
		{ 84,5 },
		{ 89,4 },
		{ 93,4 },
		{ 97,5 },
		{ 102,5}

	},

	{//forth clock digit
		{ 107,5},
		{ 112,5 },
		{ 117,5 },
		{ 122,4},
		{ 126,4 },
		{ 130,5},
		{ 135,5 }

	},
	{//fith digit - Home Score
		{ 140,7 },
		{ 147,7 },
		{ 154,7 },
		{ 161,5 },
		{ 166,5 },
		{ 171,7 },
		{ 178,7 }


	},
	{//sixth digit - Away Score
		{ 185,7},
		{ 192,7 },
		{ 199,7 },
		{ 206,5 },
		{ 211,5 },
		{ 216,7 },
		{ 223,7 }

	},
};

int LedColonMap[4] = { 67,68,71,72 };



DateTime now; // To hold the current time from the RTC

// SETUP REFRESH SPEED - HOW OFTEN SHOULD SMS ETC BE CHECKED

// int SMSCheckRate = 10; // CHECK FOR SMS EVERY 10 SECS
// int TEMPCheckRate = 600; // CHECK TEMPERATURE EVERY 10 MINS
			  
// Set-up scoreboard mode

ScoreboardModes Scoreboardmode = Reset;
TimerStatuses Timerstatus = Unset;


// MAIN SETUP FUNCTION
void setup() {

	// set up buttons

	// Set-up home button pin
	pinMode(HOMEBUTTONPIN, INPUT_PULLUP);
	debounceHome.attach(HOMEBUTTONPIN);
	debounceHome.interval(25);

	// Set-up away button pin
	pinMode(AWAYBUTTONPIN, INPUT_PULLUP);
	debounceAway.attach(AWAYBUTTONPIN);
	debounceAway.interval(25);

	// Set-up mode button pin
	pinMode(MODEBUTTONPIN, INPUT_PULLUP);
	debounceMode.attach(MODEBUTTONPIN);
	debounceMode.interval(25);

	// Set-up set button pin
	pinMode(SETBUTTONPIN, INPUT_PULLUP);
	debounceSet.attach(SETBUTTONPIN);
	debounceSet.interval(25);

	

	// SETUP SERIAL CONNECTION
	Serial.begin(9600);


	// SETUP REAL TIME CLOCK
	setupRTC();
	now = RTC.now();

	// SETUP GSM SIM

	// SET DEFAULT TIME on the Timer
	SetTimer(2100);
	Timerstatus = Set;

	// SETUP DISPLAY

	strip.begin();
	strip.setBrightness(Brightness);

	// Move to 'Normal' Mode
	Scoreboardmode = Clock;

}



void loop() {
	
	//EVERY CYCLE CHECK FOR INPUT UPDATES Buttons, SMS, RTC

	//CHECK FOR SMS
	//CheckSMS();

	//CHECK the status of each button

	debounceHome.update();
	debounceAway.update();
	debounceMode.update();
	debounceSet.update();


	bool b1 = debounceHome.fell();
	bool b2 = debounceAway.fell();
	bool b3 = debounceMode.fell();
	bool b4 = debounceSet.fell();

	

	//Get latest Time
	now = RTC.now();
	

	// Respond to buttons
	// Home Button (b1) - Same behaviour in every state
	if (b1) {
		HomeScore = HomeScore + 1; //HomeScore button pressed
		Serial.print("Home Goal");
	}

	//Away Button (b2) - same behaviour in every state
	if (b2) {
		AwayScore = AwayScore + 1; //AwayScore button pressed
		Serial.print("Away Goal");
	}


	//Mode Button (b3) - behavior depends on state
	if (b3) {
		Serial.print("Mode Button");
		// run through potential states
		switch (Scoreboardmode)
		{
		case Reset: // scoreboard just switched on or reset
		{
			// nothing to do
		}
		break; // end of Reset Case

		case Clock: //
		{
			ClockFlash = false;
			Scoreboardmode = Timer; // move to Timer Mode
		}
		break; // end of normal case

		case Timer:
		{
			ClockFlash = false;
			Scoreboardmode = Temp; // move to Temp Mode
		}
		break;
		
		case Temp:
		{
			ClockFlash = false;
			Scoreboardmode = Clock; // move to Clock Mode
		}
		break;

		case SetClockHour:
		{
			ClockFlash = true;
			ClockAddHour(); // Add 1 Hour to Current Time
		}
		break;

		case SetClockMin:
		{
			ClockFlash = true;
			ClockAddMin(); // Add 1 Min to Current Time
		}
		break;
		
		case SetTimerMin:
		{
			ClockFlash = true;
			TimerAddMin(); // Add 1 Hour to Current Time
		}
		break;
		
		case SetTimerSec:
		{
			ClockFlash = true;
			TimerAddSec(); // Add 1 Hour to Current Time
		}
		break;


		}
	}

	// Set Button Pressed
	if (b4) {
		Serial.print("Set Button");
		switch (Scoreboardmode)
		{
		case Reset: // scoreboard just switched on or reset
		{
			// nothing to do
		}
		break; // end of Reset Case

		case Clock: //
		{
			Scoreboardmode = SetClockHour; // move to Timer Mode
			ClockFlash = true;
		}
		break; // end of Clock case

		case Timer:
		{
			switch (Timerstatus)
			{
			case Unset:
			{
				// do nothing
			}
			break;
			case Set:
			{
				StartTimer();
			}
			break;
			case Running:
			{
				pauseTimer();
			}
			break;
			case Paused:
			{
				StartTimer();
			}
			break;
			case Finished:
			{
				// do nothing
			}

			}
		}
		break; // end of timer case
		
		case Temp:
		{
			// currently do nothing - will set temp later
		}
		break;

		case SetClockHour:
		{
			Scoreboardmode = SetClockMin; // Move to SetClockMin
		}
		break;

		case SetClockMin:
		{
			Scoreboardmode = SetTimerMin; // move to SetTimerMin
		}
		break;

		case SetTimerMin:
		{
			Scoreboardmode = SetTimerSec; // move to SetTimerSec
		}
		break;

		case SetTimerSec:
		{
			Scoreboardmode = Clock; // move to Clock State
			ClockFlash = false;	
		}
		break;

		}
	}
	SetClockDigits();
	SetScoreDigits();

	Serial.print(Scoreboardmode);
}
// end of Loop

void Display() {

	strip.show();
}
void SetScoreDigits() {
	// Set  the values of the score digits

	setDigit(4, HomeScore%10, ScoreDisplayColor);
	setDigit(5, AwayScore%10, ScoreDisplayColor);

}
void SetClockDigits() {

	switch (Scoreboardmode)
	{
	case Clock:
		SetTimeDigits();
		break;
	case Timer:
		SetTimerDigits();
		break;
	case Temp:
		SetTempDigits();
		break;
	case SetClockHour:
		SetTimeDigits();
		break;
	case SetClockMin:
		SetTimeDigits();
		break;
	case SetTimerMin:
		SetTimerDigits();
		break;
	case SetTimerSec:
		SetTimerDigits();
		break;
	}
}

// FUNCTION TO SET THE CLOCK DIGITS
void SetDigits(int digit0, int digit1, int digit2, int digit4, uint32_t digitcolor) {
	if (ClockFlash && (millis() % 500 < 250)) {
		digitcolor = OFF;
	}
	
	setDigit(0, digit0, digitcolor);
	setDigit(1, digit1, digitcolor);
	setDigit(2, digit2, digitcolor);
	setDigit(3, digit4, digitcolor);
} 

// FUNCTION TO SET AND SHOW ANY SINGLE DIGIT
void setDigit(int digit, int value, uint32_t color) {
	
	
	for (int seg = 0; seg < 7; seg++) {
		for (int led = LedSegmentMap[digit][seg][0]; led < LedSegmentMap[digit][seg][0]+ LedSegmentMap[digit][seg][1]; led++) {
			strip.setPixelColor(led, Font[value][seg] ? color : OFF);
		}
	}
	strip.show();
}

// FUNCTION TO SET AND SHOW THE COLON

void setColon(int vlaue, uint32_t Coloncolor) {

	for (int led = 0; led < 4; led++) {
		strip.setPixelColor(LedColonMap[led], Coloncolor);
	}
	
}

// SET A TIMER IN MEMORY TO A GIVEN NUMBER OF SECONDS
void SetTimer(int secs) {

	TimerDuration = secs;
	Timerstatus = Set;
	TimerDisplayTime = TimerDuration;

}

// FUNCTION TO START A PARTICULAR TIMER
void StartTimer() {

	now = RTC.now(); 
	long TimeNow = now.unixtime();

	TimerDuration = TimerDisplayTime;
	TimerStartTime = TimeNow;
	Timerstatus = Running;

}


// FUNCTION TO PAUSE THE TIMER
void pauseTimer() {

	Timerstatus = Paused;

}

// FUNCTION TO RESET TIMER
void resetTimer() {
	
}

// FUNCTION TO SET VALUE OF TIMER
long TimerValue() {
	
	long TimeNow = now.unixtime();

	if (Timerstatus == Running) {
		TimerDisplayTime = TimerStartTime - TimeNow + TimerDuration;
	}
	else {
		// don't update the Timer Display Time
	}

	return TimerDisplayTime;
}

// FUNCTION TO SET THE TIMER DIGITS ON THE CLOCK DISPLAY
void SetTimerDigits() {

	TimerDisplayTime = TimerValue(); //get the value to display
	
	int TimerDisplayTenMins = TimerDisplayTime / 600;
	if (TimerDisplayTenMins == 0) TimerDisplayTenMins = 10; // if zero set the Tens digit to OFF
	int TimerDisplayMins = (TimerDisplayTime / 60) % 10;
	int TimerDisplayTenSecs = ((TimerDisplayTime % 60) / 10) % 10;
	int TimerDisplaySecs = TimerDisplayTime % 10;

	if (TimerDisplayTenMins == 0) TimerDisplayTenMins = 10; // dont show a leading zero - set to 10 = OFF

		SetDigits(TimerDisplayTenMins, TimerDisplayMins, TimerDisplayTenSecs, TimerDisplaySecs, TimerDisplayColor);
	
}

// FUNCTION TO SHOW CURRENT TIME ON CLOCK DIGITS
void SetTimeDigits() {
	now = RTC.now();

	int Digit1 = (now.hour()) / 10;
	int Digit2 = now.hour() % 10;
	int Digit3 = now.minute() / 10;
	int Digit4 = now.minute() % 10;

	// Flash the COLON

	if (millis() % 500 < 250) {
		setColon(1, ClockDisplayColor);
	}  
	else
	{
		setColon(0, OFF);
	}

	SetDigits(Digit1, Digit2, Digit3, Digit4, ClockDisplayColor);
}

void SetTempDigits() {
	// show the current temperature
	temp = 10; //set sub temperature
	SetDigits(1, 0, 11, 10, TempDisplayColor);
}
// check for SMS messages
void CheckSMS() {
	//check for SMS messages
}
// Add an hour to the time
void ClockAddHour() {
	RTC.adjust(DateTime(now.year(), now.month(), now.day(), (now.hour() + 1)%24, now.minute(), now.second()));
}
// Add an hour to the time
void ClockAddMin() {
	RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), (now.minute()+1)%60, now.second()));
}
// Add a minute to the timer
void TimerAddMin() {

	int TimerSecs = TimerDisplayTime % 60;
	int TimerMins = (TimerDisplayTime - TimerSecs) / 60;
	
	TimerMins = (TimerMins + 1) % 100; // roll over at 99 mins

	TimerDuration = TimerMins*60+TimerSecs;
	TimerDisplayTime = TimerMins * 60 + TimerSecs;
}
// Add a sec to the timer
void TimerAddSec() {
	int TimerSecs = TimerDisplayTime % 60;
	int TimerMins = (TimerDisplayTime - TimerSecs) / 60;

	TimerSecs = (TimerSecs + 1) % 60; // roll over at 59 secs


	TimerDuration = TimerMins * 60 + TimerSecs;
	TimerDisplayTime = TimerMins * 60 + TimerSecs;
}

// FUNCTION TO INITIAILISE AND SET THE REAL TIME CLOCK
void setupRTC() {
	Wire.begin();
	RTC.begin();
	if (!RTC.isrunning()) {
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
	if (RTC.isrunning()) {
		Serial.println("RTC is running!");
	}
	DateTime now = RTC.now();
	long TimeNow = now.unixtime();
	SwitchOnTime = TimeNow;
}