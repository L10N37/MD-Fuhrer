/*

┌────────────────────────────────────────────────┐
│ __  __ ____    _____ _   _ _                   │
│|  \/  |  _ \  |  ___(_) (_) |__  _ __ ___ _ __ │
│| |\/| | | | | | |_  | | | | '_ \| '__/ _ \ '__|│
│| |  | | |_| | |  _| | |_| | | | | | |  __/ |   │
│|_|  |_|____/  |_|    \__,_|_| |_|_|  \___|_|   │
└────────────────────────────────────────────────┘ V1.00
VajskiDs Consoles 2024


  Only one byte of EEPROM required for settings

                          1           0
  bit 7 - language:     English    Japanese
  bit 6 - master clk:    NTSC        PAL
  bit 5 - Sub carrier:   NTSC        PAL
  bit 4 - Encoder mode:  NTSC        PAL
  bit 3 - Refresh Rate:  60Hz        50Hz
  bit 2:  Always High
  bit 1:  Always High   
  bit 0:  Always High
  // bit 0:2 - Because unwritten flash bytes are 0xFF, not 0x00 and we never touch the first 3 bits with the code

Combinations:

Straight NTSC English:      11111111 / 0xFF
Straight NTSC Japanese:     01111111 / 0x7F
Straight PAL  English:      10000111 / 0x87
Straight PAL  Japanese:     00000111 / 0x7   
NTSC 443 English:           11011111 / 0xDF    
NTSC 443 Japanese:          01011111 / 0x5F
PAL60 English:              11001111 / 0xCF
PAL60 Japanese:             01001111 / 0x4F

CXA1145M (If there are alternates, they will share the same pinout)
PIN 6: X'TAL OSC/ Sub Carrier Input via coupling capacitor, 400 to 1000mVp-p, Sine wave
PIN 7: Video output Encode mode: High for NTSC, Low for PAL

Language D9, JP1/JP2 on MD1 Mainboard, English HIGH -  JP LOW
Reset Out D12 - 5v reed relay to short reset pins, like pushing the button, covers active high and active low reset
Refresh Rate D10, JP3/JP4 on MD1 Mainboard,  Apparently Pin 46 of VDP for MDII, this works on PAL revision, does fuck all on JP VA1, 50Hz LOW, 60Hz HIGH
Encoder Mode D11, To PIN7 of encoder, High for NTSC, Low for PAL
D13 - not used

Mode selections for 3 Button Controller (PAL60, NTSC443, Straight NTSC, Straight PAL)
A + START + DN = Cycle 1: 0xF5 , Cycle 2: 0x7 - PAL60
A + START + UP = Cycle 1 : 0xF9, Cycle 2: 0xB - NTSC443
A + START + RT = Cycle 1 : 0x7D, Cycle 2: 0xF - Straight NTSC
A + START + RT = Cycle 1 : 0xBD, Cycle 2: 0xF - Straight PAL

Language Toggle: A + B + START (will detect current mode and switch)
3BTN = 
6BTN = 

Start as PAUSE for master system, coming
master system stack init, coming

*/

#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_SI5351.h>
Adafruit_SI5351 clockgen = Adafruit_SI5351();

/* Select LOW */
#define SELECT_LOW (bitRead(PINB, 0) == 0)    /* SELECT IS Digital PIN 8/ PORTB: 0 */
// UP   // D2
// DN   // D3
// A    // D4
// ST   // D5

/* Select HIGH */
#define SELECT_HIGH (bitRead(PINB, 0) == 1)  /* SELECT IS Digital PIN 8/ PORTB: 0 */
// UP   // D2
// DN   // D3
// B    // D4
// C    // D5
// LT   // D6
// RT   // D7

#define STRAIGHT_NTSC_ENG  NTSC_MASTER(), NTSC_SUBC(), ENG_MODE(), REFRESH_60(), NTSC_ENC();
#define STRAIGHT_NTSC_JP NTSC_MASTER(), NTSC_SUBC(), JP_MODE(), REFRESH_60(), NTSC_ENC();
#define STRAIGHT_PAL_ENG PAL_MASTER(), PAL_SUBC(), ENG_MODE(), REFRESH_50(), PAL_ENC();
#define STRAIGHT_PAL_JP PAL_MASTER(), PAL_SUBC(), JP_MODE(), REFRESH_50(), PAL_ENC();
#define NTSC_443_ENG NTSC_MASTER(), PAL_SUBC(), ENG_MODE(), REFRESH_60(), NTSC_ENC();
#define NTSC_443_JP NTSC_MASTER(), PAL_SUBC(), JP_MODE(), REFRESH_60(), NTSC_ENC();
#define PAL_60_ENG NTSC_MASTER(), PAL_SUBC(), ENG_MODE(), REFRESH_60(), PAL_ENC();
#define PAL_60_JP NTSC_MASTER(), PAL_SUBC(), JP_MODE(), REFRESH_60(), PAL_ENC();
#define PAL60_TOGGLE NTSC_MASTER(), PAL_SUBC(), REFRESH_60(), PAL_ENC();
#define NTSC443_TOGGLE NTSC_MASTER(), PAL_SUBC(), REFRESH_60(), NTSC_ENC();
#define STRAIGHT_PAL_TOGGLE PAL_MASTER(), PAL_SUBC(), REFRESH_50(), PAL_ENC();
#define STRAIGHT_NTSC_TOGGLE NTSC_MASTER(), NTSC_SUBC(), REFRESH_60(), NTSC_ENC();
#define JP_TOGGLE JP_MODE();
#define ENG_TOGGLE ENG_MODE();

uint8_t settings = 0xFF;
bool resetFlag = false;

// function declarations req'd as we call them during setup
void NTSC_MASTER();
void PAL_MASTER();
void NTSC_SUBC();
void PAL_SUBC();
void ENG_MODE();
void JP_MODE();
void REFRESH_60();
void REFRESH_50();
void PAL_ENC();
void NTSC_ENC();


void setup() {
  EEPROM.get(10, settings); // Store current byte from EEPROM ADDR:0 in our settings variable containing the last saved settings (0xFF is blank)
  /* Port D 2:7 as inputs */
  DDRD &= ~(B11111100);

  /* Port B all inputs */
  DDRB = 0x00;

  // set reset relay low Pin D12
  bitWrite (DDRB, 4, 1);  // set output
  bitWrite (PORTB, 4, 0); // set low

  /* SI5351 setup */
  Wire.begin();             // initialize I2C interface
  Wire.setClock(400000);    // set I2C clock speed to 400 kHz
  clockgen.begin();
  clockgen.enableOutputs(true); 

  /* init UART/ set baud rate */
  Serial.begin(2400); 


switch(settings) {

  case 0xFF:           //  Straight NTSC English, default on first boot
  STRAIGHT_NTSC_ENG;
    break;

  case 0x7F:           // Straight NTSC Japanese
  STRAIGHT_NTSC_JP
    break;

  case 0x87:           // Straight PAL English
  STRAIGHT_PAL_ENG
    break;

  case 0x7:            // Straight PAL Japanese
  STRAIGHT_PAL_JP
    break;

  case 0xDF:           // NTSC 443 English
  NTSC_443_ENG
    break;

  case 0x5F:            // NTSC 443 Japanese
  NTSC_443_JP
    break;
  
  case 0xCF:            // PAL60 English
  PAL_60_ENG
    break;

  case 0x4F:            // PAL60 Japanese
  PAL_60_JP
    break;

  default:              // EEPROM corruption, revert to US NTSC Console settings
    STRAIGHT_NTSC_ENG
    EEPROM.put(10, settings);
  }
}

void ComboCheck() {

    // SEL line cycles, 7 of 9, storage for btn presses
    uint8_t combo[6] = {0x00};

    // reset the reset flag
    resetFlag = false;

noInterrupts();
    
    while (SELECT_LOW);   // wait for a high SEL line pulse)

    while (SELECT_HIGH) { /* B + C * (3 Button Controller) */
        combo[0] = PIND;
    }

    while (SELECT_HIGH); // wait for the SEL line low pulse

    while (SELECT_LOW) { /* ST + A  (3 Button Controller)  */
        combo[1] = PIND;
    }

    while (SELECT_LOW);

    while (SELECT_HIGH) { /* ST + A  (6 Button Controller */
        combo[2] = PIND;
    }

    while (SELECT_HIGH);

    while (SELECT_LOW) { /*   (6 Button Controller)  */
        combo[3] = PIND;
    }

    while (SELECT_LOW);

    while (SELECT_HIGH) { /* (6 Button Controller */
        combo[4] = PIND;
    }

    while (SELECT_HIGH);

    while (SELECT_LOW) { /*   (3 / 6 Button Controller)  */
        combo[5] = PIND;
    }

    while (SELECT_LOW);

    
    while (SELECT_HIGH) { /* (3 / 6 Button Controller */
        combo[6] = PIND;
    }

    while (SELECT_HIGH);

interrupts();

/* 
   D0 and D1 are RX TX pins for UART, we have no control of their state during PIND captures, so we can alter the captures after 
   and set them always high. This will true up our controller capture bytes.
   https://huguesjohnson.com/programming/genesis/6button/ , we get 0xFF for all high pulses when nothing is pressed and 0x3F for low pulses (cycle 2,4,6). This coincides perfectly with the chart on this website with
   Left + Right being Low on low pulses of the select line for cycle 1 to 7, high SEL line pulses are all pins high when nothing is pressed. From here we can just monitor the debug printouts and grab our button combinations to use for what ever we like.
   Printouts won't always be 100% stable, another trade off for polling code, take what's more accurate as per docs.
*/
for (int i= 0; i < 7; i++) {
  bitSet (combo[i], 0);
  bitSet (combo[i], 1);
  }

// leave the debugging in to slow down the polling and tidy up captures, fast polling will mess up reads and you'll get false resets with A+B+C being often seen as A+B+C+ST on 6BTN controller
// the trade off for reliability with 6 BTN controllers is having to hold the combos a little longer
      Serial.print("CYCLE1 ");
      Serial.println(combo[0], HEX);
      Serial.print("CYCLE2 ");
      Serial.println(combo[1], HEX);
      Serial.print("CYCLE3 ");
      Serial.println(combo[2], HEX);
      Serial.print("CYCLE4 ");
      Serial.println(combo[3], HEX);
      Serial.print("CYCLE5 ");
      Serial.println(combo[4], HEX);
      Serial.print("CYCLE6 ");
      Serial.println(combo[5], HEX);
      Serial.print("CYCLE7 ");
      Serial.println(combo[6], HEX);
      Serial.println("\n");
      Serial.println("\n\n");
      Serial.print("Current Settings: ");
      Serial.print(settings, HEX);
      Serial.print("     Stored Settings: ");
      Serial.print(EEPROM.read(10), HEX);
      Serial.println("\n\n");


    /* 3 & 6 BTN reset detect */
    if (combo[0] == 0xCF && combo[1] == 0xF && combo[2] == 0xCF && combo[3] == 0xF && combo[4] == 0xCF && combo[5] == 0xF && combo[6] == 0xCF)
    {
        resetFlag = true;
    }
    // A + START + RT
    else if (combo[0] == 0x7F && combo[1] == 0xF ) // A + ST = 0xF Cycle 2, default 0x3F nothing pressed as per documentation + right)
    {
    STRAIGHT_NTSC_TOGGLE
    }
    else if (combo[0] == 0xBF && combo[1] == 0xF) // A + ST = 0xF Cycle 2, default 0x3F nothing pressed as per documentation + left)
    {
    STRAIGHT_PAL_TOGGLE
    }
    else if (combo[0]== 0xF7 && combo[1] ==0x7) // A + ST + DN = 0xF7 for DN on cycle 1, default 0xFF when nothing pressed as per documentation, 
    {
    PAL60_TOGGLE
    }
    else if (combo[0] == 0xFB && combo[1] == 0xB ) // 0xFB for up on cycle 1 as per documentation
    {
    NTSC443_TOGGLE
    }
    // Language Toggle (JP / ENG)
    else if (combo[5] == 0xF && combo[6] == 0xEF ) 
    {
    bool currentBitValue = bitRead(PINB, 1);
    bool newBitValue = !currentBitValue;
    bitWrite(PORTB, 1, newBitValue);
    bitWrite(settings, 7, newBitValue); //update settings byte in RAM
    delay (1000);
    }
}


// 50 Hz mode
void REFRESH_60(){
    bitWrite (DDRB, 2, 1);
    bitWrite (PORTB, 2, 1);
    // update settings byte
    bitSet(settings, 3);
}

// 60 Hz mode
void REFRESH_50(){
    bitWrite (DDRB, 2, 1);
    bitWrite (PORTB, 2, 0);
    // update settings byte
    bitClear(settings, 3);
}

// Encode PAL signal
void PAL_ENC(){
    bitWrite (DDRB, 3, 1);
    bitWrite (PORTB, 3, 0);
    // update settings byte
    bitClear(settings, 4);
}

// Encode NTSC signal
void NTSC_ENC(){
    bitWrite (DDRB, 3, 1);
    bitWrite (PORTB, 3, 1);
    // update settings byte
    bitSet(settings, 4);
  }

// NTSC SubCarrier
void NTSC_SUBC(){
    clockgen.setupPLLInt(SI5351_PLL_B, 25);  	// 25 * 25 = 625
    clockgen.setupMultisynth(1, SI5351_PLL_B, 174,200, 331);  //174.6042296072507553
    // update settings byte
    bitSet(settings, 5);
}

// PAL SubCarrier
void PAL_SUBC(){
    clockgen.setupPLLInt(SI5351_PLL_B, 36); // 900
    clockgen.setupMultisynth(1, SI5351_PLL_B, 202, 194346 , 195429); //202.9944500753476
    // update settings byte
    bitClear(settings, 5);
}

//  Set PAL mode of master clock
void PAL_MASTER() {
    clockgen.setupPLLInt(SI5351_PLL_A, 16);
    clockgen.setupMultisynth(0, SI5351_PLL_A, 7, 14, 27);
    // update settings byte
    bitClear(settings, 6);
}

//  Set NTSC mode of master clock
void NTSC_MASTER(){
    clockgen.setupPLL(SI5351_PLL_A, 30, 30, 450);
    clockgen.setupMultisynth(0, SI5351_PLL_A, 13, 5, 5);
    // update settings byte
    bitSet(settings, 6);
}

//  Set the System / Language to English mode
void ENG_MODE(){
    bitWrite (DDRB, 1, 1);
    bitWrite (PORTB, 1, 1);
    // update settings byte
    bitSet(settings, 7);
}

// Set the system / language to Japanese mode
void JP_MODE(){
    bitWrite (DDRB, 1, 1);
    bitWrite (PORTB, 1, 0);
    // update settings byte
    bitClear(settings, 7);
}


void loop() {

ComboCheck();

  /* Send a high to the 5v mini relay to throw the reset switch */ 
  if (resetFlag) {
    Serial.println ("RST");
    bitWrite (PORTB, 4, 1);     // send a high
    delay (250);                // small delay for relay to trigger
    bitWrite (PORTB, 4, 0);     // back to low
    EEPROM.update(10, settings); // "The value is written only if differs from the one already saved at the same address"
    delay (1000);               // delay to stop multiple resets if holding the combo for too long
  }
 
}
