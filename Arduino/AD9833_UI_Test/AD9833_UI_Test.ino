#include <Wire.h>
#include <stdio.h>

#define UART_TRACE  (0)

#define TITLE_STR1  ("AD9833 FG")
#define TITLE_STR2  ("20170724")

// pin assign
const int RE_A = 2;
const int RE_B = 3;
const int SW1 = 4;

const int resetPin = 17;  // analog pin 3
const int sdaPin = 18;    // analog pin 4
const int sclPin = 19;    // analog pin 5
const int i2cadr = 0x3e;
const byte contrast = 32; // 最初は大きめにして調整する

const int wfSine     = 0;
const int wfSquare   = 1;
const int wfTriangle = 2;
const int wfIndexMax = 3;

// mask of AD9833 Control Register
const uint16_t waveFormTable[] = {
  0x2000, // Sine
  0x2020, // Square
  0x2002  // Triangle
};

const char waveFormName[][20] = {
  "SIN               ",
  "SQR               ",
  "TRI               "  
};

const uint32_t frequencyTable[] = {
  1,
  2,
  5,
  10,
  20,
  50,
  100,
  500,
  1000,
  2000,
  5000,
  10000,
  20000,
  50000,
  100000,
  200000,
  500000,
  1000000,
  2000000,
  5000000,
  10000000  
};

const int frequencyIndexMax = (sizeof(frequencyTable) / sizeof(uint32_t));

int frequencyIndex = 0;
int waveFormIndex = 0;

int prevFrequencyIndex = frequencyIndex;
int prevWaveFormIndex = waveFormIndex;

const char strBuffer[80];

//--------------------------------------------------------------------------------
// Main routins
//
void setup() {
  pinMode(RE_A, INPUT_PULLUP);
  pinMode(RE_B, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP);

#if UART_TRACE
  Serial.begin(9600);
  Serial.println("AD9833 UI Test.");
  sprintf(strBuffer, "%s %s", TITLE_STR1, TITLE_STR2);
  Serial.println(strBuffer);
  delay(1000);  
#endif

  // I2C LCD
  lcd_init();
  lcd_puts(TITLE_STR1);
  //lcd_move(0x40);
  lcd_pos(1, 1);
  lcd_puts(TITLE_STR2);
}

void loop()
{
  readParams();

  uint32_t frequency = frequencyTable[frequencyIndex];
  int waveForm = waveFormTable[waveFormIndex];

#if UART_TRACE
  displayParamsSerial(frequency, waveForm);
#endif

  if (frequencyIndex != prevFrequencyIndex || waveFormIndex != prevWaveFormIndex) {
    prevFrequencyIndex = frequencyIndex;
    prevWaveFormIndex = waveFormIndex;
    displayParamsI2CLCD(frequency, waveFormIndex);
  }

//  AD9833setFrequency(frequency, waveform);
}

//--------------------------------------------------------------------------------
// UI Input functions
//
void readParams()
{
  // 周波数設定 Rotary Encoderの読み取り
  frequencyIndex += readRE();
  if (frequencyIndex <= 0) {
    frequencyIndex = 0;
  }
  else if (frequencyIndex >= frequencyIndexMax) {
    frequencyIndex = frequencyIndexMax - 1;
  }

  // 波形設定SWの読み取り
  if (digitalRead(SW1) == LOW) {
    waveFormIndex++;
    if (waveFormIndex >= wfIndexMax) {
      waveFormIndex = 0;  // wfSine;
    }
    delay(200); // (とりあえず)チャタリング防止
  }
}

// Rotary Encoderの読み取り akizuki/Alps
int readRE()
{
  static uint8_t index;
  int retVal = 0;
  index = (index << 2) | (digitalRead(RE_B) << 1) | (digitalRead(RE_A));
  index &= 0b1111;
  switch (index) {
  // 時計回り
  case 0b0111:  // 01 -> 11
    retVal = 1;
    break;
  // 反時計回り
  case 0b1101:  // 11 -> 01
    retVal = -1;
    break;
  }
  delay(1);  // (とりあえず)チャタリング防止
  return retVal;
}

//--------------------------------------------------------------------------------
// UI Display functions
//
#if UART_TRACE
void displayParamsSerial(uint32_t frequency, int waveForm)
{
  Serial.print(waveFormIndex);
  Serial.print('\t');
  Serial.print("0x");
  Serial.print(waveForm, HEX);

  Serial.print('\t');
  
  Serial.print(frequencyIndex);
  Serial.print('\t');
  Serial.println(frequency);  
}
#endif

void displayParamsI2CLCD(uint32_t frequency, int waveFormIndex)
{
  //lcd_clear();

  // 周波数表示
  lcd_pos(0, 0);
  sprintf(strBuffer, "%8luHz", frequency);
  lcd_puts(strBuffer);

  // 波形表示
  lcd_pos(1, 0);
  lcd_puts(waveFormName[waveFormIndex]);
}

//--------------------------------------------------------------------------------
// akizuki AQM0802 / aitendo SPLC792-I2C
//
void lcd_cmd(byte x)
{
  Wire.beginTransmission(i2cadr);
  Wire.write(0x00);
  Wire.write(x);
  Wire.endTransmission();
}

void lcd_data(byte x)
{
  Wire.beginTransmission(i2cadr);
  Wire.write(0x40);
  Wire.write(x);
  Wire.endTransmission();
}

void lcd_puts(const char *s)
{
  while(*s) lcd_data(*s++);
}

void lcd_init()
{
  // reset
  delay(500);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(1);
  digitalWrite(resetPin, HIGH);
  delay(10);
  // LCD initialize
  delay(40);
  Wire.begin();
  lcd_cmd(0x38); // function set
  lcd_cmd(0x39); // function set
  lcd_cmd(0x14); // interval osc
  lcd_cmd(0x70 | (contrast & 15)); // contrast low
  lcd_cmd(0x5c | (contrast >> 4 & 3)); // contrast high / icon / power
  lcd_cmd(0x6c); // follower control
  delay(300);
  lcd_cmd(0x38); // function set
  lcd_cmd(0x0c); // display on
  lcd_cmd(0x01); // clear display
  delay(2);
}

/*
void lcd_move(byte pos){
  lcd_cmd(0x80 | pos);
}
*/
void lcd_pos(byte raw, byte col) {
  lcd_cmd(0x80 | ((raw & 0x01) << 6) | col);
}

void lcd_clear() {
  lcd_cmd(0x01);
}


/*
void AD9833setFrequency(uint32_t frequency, uint16_t Waveform) {
  Serial.print("0x");
  Serial.print(Waveform, HEX);
  Serial.print('\t');
  Serial.println(frequency);  
}
*/


