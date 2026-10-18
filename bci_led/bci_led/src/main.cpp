#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define LED_PIN 4
#define NUM_LEDS 72
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ------------------------ CONSTANTS ------------------------
const int placesCount = 6;
const int placeLedCount = 12;
const int placeLedStartIndex[placesCount] = {0,12,24,36,48,60};

// ------------------------ VARIABLES ------------------------
bool mode1Active = false;
bool mode2Active = false;
bool running = false;

float mode1Freq[placesCount];
int mode1FlickDur;
int mode1PauseDur;
uint8_t mode1Brightness;
uint8_t mode1Color[3];
int mode1Sequence[placesCount];

float mode2Freq[5];
int mode2FlickDur;
int mode2PauseDur;
uint8_t mode2Brightness;
uint8_t mode2Color[3];

unsigned long lastFlickerTime = 0;
int currentPlace = 0;

#define SERIAL_BUFFER_SIZE 128
char serialBuffer[SERIAL_BUFFER_SIZE];
int bufferIndex = 0;

// ------------------------ EEPROM STRUCT ------------------------
struct Settings {
  float mode1Freq[placesCount];
  int mode1FlickDur;
  int mode1PauseDur;
  uint8_t mode1Brightness;
  uint8_t mode1Color[3];
  int mode1Sequence[placesCount];

  float mode2Freq[5];
  int mode2FlickDur;
  int mode2PauseDur;
  uint8_t mode2Brightness;
  uint8_t mode2Color[3];

  uint32_t signature;
};

Settings config;

// ------------------------ FUNCTION PROTOTYPES ------------------------
void trim(char* str);
bool parseColor(char* col, uint8_t* rgb);
bool validateMode1Params(float* f, int flickDur, int pauseDur, int bright, char* color);
bool validateMode2Params(float* f, int flickDur, int pauseDur, int bright, char* color);

void showFlowingRadiant();
void parseSerialMessage(char* msg);

void startMode1();
void startMode2();
void stopAll();

void flickerMode1();
void flickerMode2();

void showPlace(int place, bool onOff);

void saveSettings();
void loadSettings();
void setDefaults();

// -------------------------------------------------------
// EEPROM SAVE
// -------------------------------------------------------
void saveSettings() {
  config.signature = 0xAABBCCDD;

  memcpy(config.mode1Freq, mode1Freq, sizeof(mode1Freq));
  config.mode1FlickDur = mode1FlickDur;
  config.mode1PauseDur = mode1PauseDur;
  config.mode1Brightness = mode1Brightness;
  memcpy(config.mode1Color, mode1Color, 3);
  memcpy(config.mode1Sequence, mode1Sequence, sizeof(mode1Sequence));

  memcpy(config.mode2Freq, mode2Freq, sizeof(mode2Freq));
  config.mode2FlickDur = mode2FlickDur;
  config.mode2PauseDur = mode2PauseDur;
  config.mode2Brightness = mode2Brightness;
  memcpy(config.mode2Color, mode2Color, 3);

  EEPROM.put(0, config);
  EEPROM.commit();

  Serial.println("EEPROM Saved");
}

// -------------------------------------------------------
// EEPROM DEFAULTS
// -------------------------------------------------------
void setDefaults() {
  for (int i=0;i<placesCount;i++) mode1Freq[i] = 5.0f;
  mode1FlickDur = 3;
  mode1PauseDur = 2;
  mode1Brightness = 128;
  mode1Color[0] = 255; mode1Color[1] = 0; mode1Color[2] = 0;

  for (int i=0;i<placesCount;i++) mode1Sequence[i] = i;

  for (int i=0;i<5;i++) mode2Freq[i] = 5.0f;
  mode2FlickDur = 3;
  mode2PauseDur = 2;
  mode2Brightness = 128;
  mode2Color[0] = 0; mode2Color[1] = 255; mode2Color[2] = 0;

  saveSettings();
}

// -------------------------------------------------------
// EEPROM LOAD
// -------------------------------------------------------
void loadSettings() {
  EEPROM.get(0, config);

  if (config.signature != 0xAABBCCDD) {
    Serial.println("EEPROM empty or corrupted. Loading defaults...");
    setDefaults();
    return;
  }

  memcpy(mode1Freq, config.mode1Freq, sizeof(mode1Freq));
  mode1FlickDur = config.mode1FlickDur;
  mode1PauseDur = config.mode1PauseDur;
  mode1Brightness = config.mode1Brightness;
  memcpy(mode1Color, config.mode1Color, 3);
  memcpy(mode1Sequence, config.mode1Sequence, sizeof(mode1Sequence));

  memcpy(mode2Freq, config.mode2Freq, sizeof(mode2Freq));
  mode2FlickDur = config.mode2FlickDur;
  mode2PauseDur = config.mode2PauseDur;
  mode2Brightness = config.mode2Brightness;
  memcpy(mode2Color, config.mode2Color, 3);
}

// ------------------------ SETUP ------------------------
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  EEPROM.begin(512);

  loadSettings();
  showFlowingRadiant();

  Serial.println("Device initialized and STOP state");
}

// ------------------------ MAIN LOOP ------------------------
void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        serialBuffer[bufferIndex] = '\0';
        trim(serialBuffer);
        parseSerialMessage(serialBuffer);
        bufferIndex = 0;
      }
    } else if (bufferIndex < SERIAL_BUFFER_SIZE - 1) {
      serialBuffer[bufferIndex++] = c;
    }
  }

  if (running) {
    if (mode1Active){
      flickerMode1();
    } 
    else if (mode2Active) flickerMode2();
  }
}

// ------------------------ PARSER ------------------------
void parseSerialMessage(char* msg) {

  if (strcmp(msg, "STOP") == 0) {
    stopAll();
    Serial.println("Stopped");
    return;
  }

  if (strcmp(msg, "FETCH") == 0) {
    Serial.print("MODE1 freq: ");
    for (int i=0;i<placesCount;i++) { Serial.print(mode1Freq[i]); Serial.print(","); }
    Serial.println();

    Serial.print("FlickDur,PauseDur,Brightness: ");
    Serial.print(mode1FlickDur); Serial.print(",");
    Serial.print(mode1PauseDur); Serial.print(",");
    Serial.println(mode1Brightness);

    Serial.print("Color RGB: ");
    Serial.print(mode1Color[0]); Serial.print(",");
    Serial.print(mode1Color[1]); Serial.print(",");
    Serial.println(mode1Color[2]);

    Serial.print("Sequence: ");
    for (int i=0;i<placesCount;i++) { Serial.print(mode1Sequence[i]); Serial.print(","); }

    Serial.println("\nMODE2 freq: ");
    for (int i=0;i<5;i++) { Serial.print(mode2Freq[i]); Serial.print(","); }
    Serial.println();

    Serial.print("FlickDur,PauseDur,Brightness: ");
    Serial.print(mode2FlickDur); Serial.print(",");
    Serial.print(mode2PauseDur); Serial.print(",");
    Serial.println(mode2Brightness);

    Serial.print("Color RGB: ");
    Serial.print(mode2Color[0]); Serial.print(",");
    Serial.print(mode2Color[1]); Serial.print(",");
    Serial.println(mode2Color[2]);
    return;
  }

  // ------------------ START MODE ------------------
  if (strcmp(msg, "STARTM1") == 0) { startMode1(); Serial.println("Mode 1 started"); return; }
  if (strcmp(msg, "STARTM2") == 0) { startMode2(); Serial.println("Mode 2 started"); return; }

  // ------------------ SEQUENCE ------------------
  if (strncmp(msg, "M1SEQ", 5) == 0) {
    char* token = strtok(msg + 6, ",");
    int seqTemp[placesCount];
    int count=0;
    bool valid=true;

    while (token != NULL && count < placesCount) {
      int v = atoi(token);
      if (v < 0 || v >= placesCount) valid = false;
      seqTemp[count++] = v;
      token = strtok(NULL, ",");
    }

    if (count != placesCount) valid = false;

    for (int i=0;i<placesCount;i++)
      for (int j=i+1;j<placesCount;j++)
        if (seqTemp[i] == seqTemp[j]) valid = false;

    if (!valid) { Serial.println("Invalid sequence"); return; }

    memcpy(mode1Sequence, seqTemp, sizeof(mode1Sequence));
    saveSettings();
    Serial.println("Sequence updated");
    return;
  }

  // ------------------ MODE 1 PARAMS ------------------
  if (strncmp(msg, "M1", 2) == 0) {
    char* token = strtok(msg + 3, ",");
    float freqTemp[placesCount];
    int i = 0;

    while (token != NULL && i < placesCount) {
      freqTemp[i++] = atof(token);
      token = strtok(NULL, ",");
    }

    if (i != placesCount) { Serial.println("Invalid frequency count"); return; }

    char* flickStr = token;
    char* pauseStr = flickStr ? strtok(NULL, ",") : NULL;
    char* brightStr = pauseStr ? strtok(NULL, ",") : NULL;
    char* colorStr = brightStr ? strtok(NULL, ",") : NULL;

    if (!flickStr || !pauseStr || !brightStr || !colorStr) {
      Serial.println("Missing parameters");
      return;
    }

    int flickTmp = atoi(flickStr);
    int pauseTmp = atoi(pauseStr);
    int brightTmp = atoi(brightStr);

    if (!validateMode1Params(freqTemp, flickTmp, pauseTmp, brightTmp, colorStr)) {
      Serial.println("Invalid parameters");
      return;
    }

    memcpy(mode1Freq, freqTemp, sizeof(mode1Freq));
    mode1FlickDur = flickTmp;
    mode1PauseDur = pauseTmp;
    mode1Brightness = brightTmp;
    parseColor(colorStr, mode1Color);

    saveSettings();
    Serial.println("Mode 1 parameters updated");
    return;
  }

  // ------------------ MODE 2 PARAMS ------------------
  if (strncmp(msg, "M2", 2) == 0) {
    char* token = strtok(msg + 3, ",");
    float freqTemp[5];
    int i = 0;

    while (token != NULL && i < 5) {
      freqTemp[i++] = atof(token);
      token = strtok(NULL, ",");
    }

    if (i != 5) { Serial.println("Invalid frequency count"); return; }

    char* flickStr = token;
    char* pauseStr = flickStr ? strtok(NULL, ",") : NULL;
    char* brightStr = pauseStr ? strtok(NULL, ",") : NULL;
    char* colorStr = brightStr ? strtok(NULL, ",") : NULL;

    if (!flickStr || !pauseStr || !brightStr || !colorStr) {
      Serial.println("Missing parameters");
      return;
    }

    int flickTmp = atoi(flickStr);
    int pauseTmp = atoi(pauseStr);
    int brightTmp = atoi(brightStr);

    if (!validateMode2Params(freqTemp, flickTmp, pauseTmp, brightTmp, colorStr)) {
      Serial.println("Invalid parameters");
      return;
    }

    memcpy(mode2Freq, freqTemp, sizeof(mode2Freq));
    mode2FlickDur = flickTmp;
    mode2PauseDur = pauseTmp;
    mode2Brightness = brightTmp;
    parseColor(colorStr, mode2Color);

    saveSettings();
    Serial.println("Mode 2 parameters updated");
    return;
  }

  Serial.println("Unknown command");
}

// -------------------------------------------------------

void showFlowingRadiant() {
  for (int p=0;p<3;p++) {
    for (int i=0;i<NUM_LEDS;i++) {
      strip.clear();
      strip.setPixelColor(i, strip.Color(255,255,255));
      strip.show();
      delay(30);
    }
  }
  strip.clear();
  strip.show();
}

void startMode1() {
  mode1Active=true;
  mode2Active=false;
  running=true;
  currentPlace=0;
  lastFlickerTime=millis();
}

void startMode2() {
  mode2Active=true;
  mode1Active=false;
  running=true;
  currentPlace=0;
  lastFlickerTime=millis();
}

void stopAll() {
  running=false;
  mode1Active=false;
  mode2Active=false;
  strip.clear();
  strip.show();
}

void flickerMode1() {
  // --- Static variables to hold state ---
  static bool flickOn = false;
  static bool isPaused = true;       // NEW: Start in a paused state
  static unsigned long flickStart = 0;    // Times the *total* duration
  
  // (Assuming lastFlickerTime is global or static, 
  // otherwise it must be static here)
  // static unsigned long lastFlickerTime = 0; // Tracks flick/pause timing

  // --- Current time and flicker period ---
  unsigned long now = millis();
  float freq = mode1Freq[mode1Sequence[currentPlace]];
  unsigned long halfPeriod = (unsigned long)(1000.0 / freq / 2.0);

  // -------------------------------------------
  // STATE 1: We are PAUSED, wait for pause to end
  // -------------------------------------------
  if (isPaused) {
    if (now - lastFlickerTime >= (unsigned long)(mode1PauseDur * 1000.0)) {
      // Pause is over, start flickering
      isPaused = false;
      flickOn = true;
      flickStart = now;       // !IMPORTANT: Set duration start time
      lastFlickerTime = now;  // Set pulse start time
      showPlace(mode1Sequence[currentPlace], true);
      // Serial.print("MODE1 Starting place: ");
      // Serial.println(mode1Sequence[currentPlace]);
    }
    return; // Do nothing else if still paused
  }

  // -------------------------------------------
  // STATE 2: We are ACTIVELY FLICKERING
  // -------------------------------------------
  
  // Check 1: Is the *total* flicker duration over?
  if (now - flickStart >= (unsigned long)(mode1FlickDur * 1000.0)) {
    // Duration is over. Stop and go back to PAUSED.
    isPaused = true;
    flickOn = false;
    lastFlickerTime = now; // Start timing the PAUSE
    showPlace(mode1Sequence[currentPlace], false); // Ensure light is off
    
    currentPlace = (currentPlace + 1) % placesCount; // Move to next place
    Serial.print("MODE1 Pausing before place: ");
    Serial.println(mode1Sequence[currentPlace]);
    return;
  }

  // Check 2: Is it time to toggle the flicker (on/off)?
  if (now - lastFlickerTime >= halfPeriod) {
    flickOn = !flickOn;     // Toggle the state
    lastFlickerTime = now;  // Mark the time of this toggle
    showPlace(mode1Sequence[currentPlace], flickOn);
    return;
  }

  // -------------------------------------------
  // STATE 3: WAITING (NTD)
  // -------------------------------------------
  // If we are here, it means we are:
  // 1. In the PAUSED state, but the pause time isn't over.
  // 2. In the FLICKERING state, but the halfPeriod isn't over.
  // This "NTD" block is now only hit when it's correctly waiting.
  else {
    //print NTD
    // Serial.print("NTD");

    //print the conditions for debugging
    // Serial.print(" Now: ");
    // Serial.println(now);
    // Serial.print(" LastFlickerTime: ");
    // Serial.println(lastFlickerTime);
    // Serial.print(" FlickStart: ");
    // Serial.println(flickStart);
    // Serial.print(" IsPaused: ");
    // Serial.println(isPaused);
    // Serial.print(" FlickOn: ");
    // Serial.println(flickOn);
    return;
  }
}



void flickerMode2() {
  static bool flickOn=false;
  unsigned long now=millis();
  unsigned long half = (unsigned long)(1000.0/mode2Freq[currentPlace]/2.0);

  if (!flickOn && now-lastFlickerTime >= mode2PauseDur*1000UL) {
    flickOn=true;
    lastFlickerTime=now;
    Serial.print("MODE2 Flickering group: ");
    Serial.println(currentPlace);   // 0,1 = rows; 2,3,4 = columns

    if (currentPlace < 2) {
      for (int c=0;c<3;c++) showPlace(currentPlace*3 + c, true);
    } else {
      int col = currentPlace - 2;
      for (int r=0;r<2;r++) showPlace(r*3 + col, true);
    }
  }
  else if (flickOn && now-lastFlickerTime >= half) {
    flickOn=false;
    lastFlickerTime=now;

    if (currentPlace < 2) {
      for (int c=0;c<3;c++) showPlace(currentPlace*3 + c, false);
    } else {
      int col = currentPlace - 2;
      for (int r=0;r<2;r++) showPlace(r*3 + col, false);
    }

    currentPlace = (currentPlace+1) % 5;
  }
}

void showPlace(int place, bool onOff) {

  
  // Serial.println(place);
  
  uint32_t col = strip.Color(0,0,0);

  if (onOff) {
    // Print MODE + PLACE every time LEDs turn ON
    // if (mode1Active) {
    //   Serial.print("MODE1 Flickering place: ");
    //   Serial.println(place);
    // } else if (mode2Active) {
    //   Serial.print("MODE2 Flickering place: ");
    //   Serial.println(place);
    // }

    

    // existing color logic
    if (mode1Active)
      col = strip.Color(
        (uint8_t)(mode1Color[0] * (mode1Brightness / 255.0f)),
        (uint8_t)(mode1Color[1] * (mode1Brightness / 255.0f)),
        (uint8_t)(mode1Color[2] * (mode1Brightness / 255.0f))
      );
    else
      col = strip.Color(
        (uint8_t)(mode2Color[0] * (mode2Brightness / 255.0f)),
        (uint8_t)(mode2Color[1] * (mode2Brightness / 255.0f)),
        (uint8_t)(mode2Color[2] * (mode2Brightness / 255.0f))
      );
  }

  int start = placeLedStartIndex[place];
  for (int i=0; i < placeLedCount; i++)
    strip.setPixelColor(start + i, col);

  strip.show();
}


void trim(char* str) {
  int len=strlen(str);
  while (len>0 && (str[len-1]=='\n' || str[len-1]=='\r' || str[len-1]==' '))
    str[--len]='\0';
}

bool parseColor(char* col, uint8_t* rgb) {
  if (strcmp(col,"RED")==0){ rgb[0]=255; rgb[1]=0; rgb[2]=0; return true; }
  if (strcmp(col,"GREEN")==0){ rgb[0]=0; rgb[1]=255; rgb[2]=0; return true; }
  if (strcmp(col,"BLUE")==0){ rgb[0]=0; rgb[1]=0; rgb[2]=255; return true; }
  return false;
}

bool validateMode1Params(float* f, int flickDur, int pauseDur, int bright, char* color) {
  for (int i=0;i<placesCount;i++) if (f[i]<2.0 || f[i]>20.0) return false;
  if (flickDur<1 || flickDur>10) return false;
  if (pauseDur<1 || pauseDur>10) return false;
  if (bright<0 || bright>255) return false;

  uint8_t dummy[3];
  return parseColor(color, dummy);
}

bool validateMode2Params(float* f, int flickDur, int pauseDur, int bright, char* color) {
  for (int i=0;i<5;i++) if (f[i]<2.0 || f[i]>20.0) return false;
  if (flickDur<1 || flickDur>10) return false;
  if (pauseDur<1 || pauseDur>10) return false;
  if (bright<0 || bright>255) return false;

  uint8_t dummy[3];
  return parseColor(color, dummy);
}
