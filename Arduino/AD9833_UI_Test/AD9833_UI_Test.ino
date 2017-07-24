#include <Wire.h>

#define UART_TRACE  (1)

// pin assign
const int RE_A = 2;
const int RE_B = 3;
const int SW1 = 4;

enum waveForm_t { wfSine, wfSquare, wfTriangle, wfIndexMax };

// mask of AD9833 Control Register
const uint16_t waveFormTable[] = {
  0x2000, // Sine
  0x2020, // Square
  0x2002  // Triangle
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
  1000000,
  2000000,
  5000000,
  10000000  
};

const int frequencyIndexMax = (sizeof(frequencyTable) / sizeof(uint32_t));

int frequencyIndex = 0;
int waveFormIndex = 0;

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
#endif
}

void loop()
{
  readParams();

  uint32_t frequency = frequencyTable[frequencyIndex];
  waveForm_t waveForm = waveFormTable[waveFormIndex];

#if UART_TRACE
  displayParamsSerial(frequency, waveForm);
#endif

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
void displayParamsSerial(uint32_t frequency, waveForm_t waveForm)
{
  Serial.print("0x");
  Serial.print(waveForm, HEX);
  Serial.print('\t');
  Serial.println(frequency);  
}
#endif

void displayParamsI2CLCD(uint32_t frequency, waveForm_t waveForm)
{

}

/*
void AD9833setFrequency(uint32_t frequency, uint16_t Waveform) {
  Serial.print("0x");
  Serial.print(Waveform, HEX);
  Serial.print('\t');
  Serial.println(frequency);  
}
*/


