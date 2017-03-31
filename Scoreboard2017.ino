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

#define N_LEDS 230

// Define colors to make setting the LEDs easier
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF
#define OFF    0X000000


// define global variables

int Brightness = 100;
long TimerStartTime = 0;
long TimerDuration = 0;
long TimerDisplayTime = 0;
long TimeNow = 0;
long Timer1 = 0;
long SwitchOnTime;
int HomeScore = 0;
int AwayScore = 0;
long temp = 0;


boolean ClockFlash = false;

enum  ScoreboardModes { Reset, Clock, Timer, Temp, SetClockHour, SetClockMin, SetTimerMin, SetTimerSec };
enum TimerStatuses { Unset, Set, Running, Paused, Finished};

uint32_t ClockDisplayColor = BLUE;
uint32_t TimerDisplayColor = GREEN;
uint32_t ScoreDisplayColor = RED;

RTC_DS1307 RTC;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, LEDPIN, NEO_GRB + NEO_KHZ800);


// Font definitions ////////////////////////////
// 7 segments set up as

//  33333
//  4   2
//  11111
//  5   7
//  66666

int Font[12][7] = {
	{ 0,1,1,1,1,1,1 }, //value 0
	{ 0,1,0,0,0,0,1 }, //value 1
	{ 1,1,1,0,1,1,0 }, //value 2
	{ 1,1,1,0,0,1,1 }, //value 3
	{ 1,1,0,1,0,0,1 }, //value 4
	{ 1,0,1,1,0,1,1 }, //value 5
	{ 1,0,0,1,1,1,1 }, //value 6
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




int LedSegmentMap[6][7][8] = {
	{//first clock digit
		{ 0,1,2,3,4,4,4,4 }, // segment 0
		{ 5,6,7,8,9,9,9,9 }, // segment 1
		{ 10,11,12,13,14,14,14,14 }, // segment 2
		{ 15,16,17,18,18,18,18,18 },// segment 3
		{ 19,20,21,22,22,22,22,22 }, // segment 4
		{ 23,24,25,26,27,27,27,27 }, // segment 5
		{ 28,29,30,31,32,32,32,32 } // segment 6
	},



	{//second clock digit
		{ 33,34,35,36,37,37,37,37 },
		{ 38,39,40,41,42,42,42,42 },
		{ 43,44,45,46,47,47,47,47 },
		{ 48,49,50,51,51,51,51,51 },
		{ 52,53,54,55,55,55,55,55 },
		{ 56,57,58,59,60,60,60,60 },
		{ 61,62,63,64,65,65,65,65 }

	},
	{//third clock digit
		{ 74,75,76,77,78,78,78,78 },
		{ 79,80,81,82,83,83,83,83 },
		{ 84,85,86,87,88,88,88,88 },
		{ 89,90,91,92,92,92,92,92 },
		{ 93,94,95,96,96,96,96,96 },
		{ 97,98,99,100,101,101,101,101 },
		{ 102,103,104,105,106,106,106,106 }

	},

	{//forth clock digit
		{ 107,108,109,110,111,111,111,111 },
		{ 112,113,114,115,116,116,116,116 },
		{ 117,118,119,120,121,121,121,121 },
		{ 122,123,124,125,125,125,125,125 },
		{ 126,127,128,129,129,129,129,129 },
		{ 130,131,132,133,134,134,134,134 },
		{ 135,136,137,138,139,139,139,139 }


	},
	{//fith digit - Home Score
		{ 140,141,142,143,144,145,146,146 },
		{ 147,148,149,150,151,152,153,153 },
		{ 154,155,156,157,158,159,160,160 },
		{ 161,162,163,164,165,165,165,165 },
		{ 166,167,168,169,170,170,170,170 },
		{ 171,172,173,174,175,176,177,177 },
		{ 178,179,180,181,182,183,184,184 }


	},
	{//sixth digit - Away Score
		{ 185,186,187,188,189,190,191,191 },
		{ 192,193,194,195,196,197,198,198 },
		{ 199,200,201,202,203,204,205,205 },
		{ 206,207,208,209,210,210,210,210 },
		{ 211,212,213,214,215,215,215,215 },
		{ 216,217,218,219,220,221,222,222 },
		{ 223,224,225,226,227,228,229,229 }

	},
};

int LedColonMap[] = { 67,68,71,72,72,72,72,72 };



DateTime now; // To hold the current time from the RTC

// SETUP REFRESH SPEED - HOW OFTEN SHOULD SMS ETC BE CHECKED

int SMSCheckRate = 10; // CHECK FOR SMS EVERY 10 SECS
int TEMPCheckRate = 600; // CHECK TEMPERATURE EVERY 10 MINS
			  
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
			ClockAddHour(); // Add 1 Hour to Current Time
		}
		break;

		case SetClockMin:
		{
			ClockAddMin(); // Add 1 Min to Current Time
		}
		break;
		
		case SetTimerMin:
		{
			TimerAddMin(); // Add 1 Hour to Current Time
		}
		break;
		
		case SetTimerSec:
		{
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

	setDigit(4, HomeScore, ScoreDisplayColor);
	setDigit(5, AwayScore, ScoreDisplayColor);

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
		ShowTemp();
		break;
	}
}

// FUNCTION TO SET THE CLOCK DIGITS
void SetDigits(int digit0, int digit1, int digit2, int digit4, uint32_t digitcolor) {
	setDigit(0, digit0, digitcolor);
	setDigit(1, digit1, digitcolor);
	setDigit(2, digit2, digitcolor);
	setDigit(3, digit4, digitcolor);
} 

// FUNCTION TO SET AND SHOW ANY SINGLE DIGIT
void setDigit(int digit, int value, uint32_t color) {
	int ledsPerSeg = 8;
	
	for (int seg = 0; seg < 7; seg++) {
		for (int led = 0; led < ledsPerSeg; led++) {
			strip.setPixelColor(LedSegmentMap[digit][seg][led], Font[value][seg] ? color : OFF);
		}
	}
	strip.show();
}

// FUNCTION TO SET AND SHOW THE COLON

void setColon(int vlaue, uint32_t Coloncolor) {

	for (int led = 0; led < 8; led++) {
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

	if (millis() % 1000 < 100) {
		setColon(1, ClockDisplayColor);
	}
	else
	{
		setColon(0, OFF);
	}

	SetDigits(Digit1, Digit2, Digit3, Digit4, ClockDisplayColor);
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


// Set the clock digits

void ShowScore() {
	// Set the scoreboard digits
}
void ShowTemp() {
	// show the current temperature
}
// check for SMS messages
void CheckSMS() {
	//check for SMS messages
}
// Add an hour to the time
void ClockAddHour() {
	RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 1, now.minute(), now.second()));
}
// Add an hour to the time
void ClockAddMin() {
	RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute()+1, now.second()));
}
// Add an hour to the time
void TimerAddMin() {

	// this isnt going to work as Display value for the timer isnt held directly - need to update start time and duration
	TimerDuration = TimerDuration + 60;
}
// Add an hour to the time
void TimerAddSec() {
	TimerDuration = TimerDuration + 1;
}
