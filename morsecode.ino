#include <LiquidCrystal.h>
// These are to map morse code to characters (last two are for the custom glyphs so ignore them)
const char* codes[38] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", "......", ".....-"};
const char values[38] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '!', '@'};
//Initialize LCD
LiquidCrystal lcd(3, 2, 4, 5, 6, 7);

int btn = 8;						//Morse input btn pin
int buzzer = 9;					//Buzzer output
int spaceBtn = 10;			//Space/End char btn pin
int dotLED = 11;				//1st LED pin
int dashLED = 12;				//2nd LED pin
int resetBtn = 13;			//Reset btn pin

int downStart = 0;			//Timestamp at first press of morse button
int spaceDownStart = 0;	//Timestamp at first press of space button
bool down = false;			//Tracks state of morse btn
bool spaceDown = false;	//Track state of space btn

int dashTime = 200; 		//Amount of time required to count as a dash instead of a dot
int debounce = 20;			//Avoid avoid glitches
String message;

// This isnt part of morse code but I added a smiley "......" and a heart ".....-"
// I did this because I was playing with the capibilities of the LCD so you can ignore it
char smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};
char heart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
};

//OUTPUT to display (checked each char because ! and @ are supposted to print a smiley...
// and a heart, normally you can just print the whole translated string)
void display(String msg) {
	int len = msg.length();
	int begin = len-32 > 0 ? len-32 : 0;
	lcd.clear();
	lcd.setCursor(0,0);
	for(int i = begin;i < len;++i) {
		char c = msg.charAt(i);
		lcd.setCursor((i-begin)%16,(i > begin + 15 ? 1 : 0));
		if(c == '!') {
			lcd.write(byte(0));
		}
		else if(c == '@') {
			lcd.write(byte(1));
		}
		else {
			lcd.print(c);
		}
	}
}

//Maps morse code to its approprate char
char mapChar(const char* code) {
	for(int i = 0;i < 38;++i) {
		if(strncmp(code,codes[i],6) == 0) {
			return values[i];
		}
	}
	return '*';
}

// Function that returns a translated string from morse
String translate(String msg) {
	String translated = "";
	char current[6];
	memset(current, 0, 6);
	uint8_t pos = 0;
	for(int i = 0;i < msg.length();++i) {
		char c = msg.charAt(i);
		if(c == ' ') {
			if(pos != 0) {translated += mapChar(current);}
			else {translated += " ";}
			pos = 0;
			memset(current, 0, 6);
		}
		else if(c == '*') {
			if(pos != 0) {translated += mapChar(current);}
			translated += " ";
			pos = 0;
			memset(current, 0, 6);
		}
		else if(pos < 6) {
			current[pos] = c;
			pos ++;
		}
	}
	return translated;
}

void setup() {
	//Start lcd (cols,rows)
	lcd.begin(16,2);
	//Add custom chars (ignore)
	lcd.createChar(0, smiley);
	lcd.createChar(1, heart);
	//Set pin modes
	pinMode(btn, INPUT);
	pinMode(spaceBtn, INPUT);
	pinMode(resetBtn, INPUT);
	pinMode(dashLED, OUTPUT);
	pinMode(dotLED, OUTPUT);
	pinMode(buzzer, OUTPUT);
}

void loop() {
	//Read current btn values
	bool tmpdown = digitalRead(btn);
	bool tmpdownstart = digitalRead(spaceBtn);
	//Get current time to check length of press
	int currentTime = millis();


	if(tmpdown && !down) {	// This checks for morse btn push down
		down = true;
		downStart = currentTime;
	}
	else if(!tmpdown && down) {	// This checks for morse btn release
		down = false;
		if(currentTime - downStart > dashTime) {
			message += "-";
			digitalWrite(dotLED, 1);
			digitalWrite(dashLED, 1);
		}
		else if(currentTime - downStart > debounce) {
			message += ".";
			digitalWrite(dotLED, 1);
			digitalWrite(dashLED, 0);
		}
	}
	else {
		if(!down && tmpdownstart && !spaceDown) {	//This checks for space btn push down
			spaceDownStart = currentTime;
			spaceDown = true;
			digitalWrite(dotLED, 0);
			digitalWrite(dashLED, 0);
		}
		else if(!down && !tmpdownstart && spaceDown) {	//This checks for space btn release
			spaceDown = false;
			if(currentTime - spaceDownStart > dashTime) {
				message += "*";
				digitalWrite(dashLED, 0);
				digitalWrite(dotLED, 0);
			}
			else if(currentTime - spaceDownStart > debounce) {
				message += " ";
				digitalWrite(dotLED, 0);
				digitalWrite(dashLED, 0);
			}
			//Translate and display message
			display(translate(message));
		}
	}
	//Check for reset
	if(digitalRead(resetBtn)) {
		message = "";
		display("");
	}
	//Play tone if morse btn is pressed
	if(tmpdown) {
		tone(buzzer, 261);
	}
	else {
		noTone(buzzer);
	}
}

