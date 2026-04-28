#include <MD_MAX72xx.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>

SoftwareSerial bt(2,3);
char m;

bool md = true;
bool lr = true;
int f = 0;
int max_e=11;
const int statePin = 4;

bool b = true;
bool c = true;

// Motor Pins (Fixed Setup)
#define M1A A2
#define M1B A3
#define M2A A4
#define M2B A5

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 16
#define CS_PIN 10

MD_MAX72XX mx(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Flip settings (change only if orientation is wrong)
const bool FLIP_X = false;    // left-right mirror
const bool FLIP_Y = true;    // bottom-top flip

// -------------------- 32x32 BITMAPS IN PROGMEM (FLASH) --------------------
// Convention: MSB (bit 31) = leftmost pixel (x=0), LSB (bit 0) = rightmost pixel (x=31)

// --- AUTO-GENERATED AND VERIFIED C ARRAYS --- //

const uint32_t look_left[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x08081010,
    0x0B881710,
    0x0A881510,
    0x0B881710,
    0x08081010,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00C00300,
    0x00A00500,
    0x00DFFB00,
    0x00600600,
    0x00381C00,
    0x000FF000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t look_right[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x08081010,
    0x08E811D0,
    0x08A81150,
    0x08E811D0,
    0x08081010,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00C00300,
    0x00A00500,
    0x00DFFB00,
    0x00600600,
    0x00381C00,
    0x000FF000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t look_up[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x08081010,
    0x08081010,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00C00300,
    0x00A00500,
    0x00DFFB00,
    0x00600600,
    0x00381C00,
    0x000FF000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t normal[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x08081010,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x08081010,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x007FFE00,
    0x007FFE00,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t sad[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x07F00FE0,
    0x08081010,
    0x03E007C0,
    0x04100820,
    0x08081010,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x000FF000,
    0x00100800,
    0x0027E400,
    0x00581A00,
    0x00600600,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t look_down[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x08081010,
    0x08081010,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00C00300,
    0x00A00500,
    0x00DFFB00,
    0x00600600,
    0x00381C00,
    0x000FF000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t happy[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x07F00FE0,
    0x08081010,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00C00300,
    0x00A00500,
    0x00DFFB00,
    0x00600600,
    0x00381C00,
    0x000FF000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t confused[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000380,
    0x00000440,
    0x07C00820,
    0x00700000,
    0x000807C0,
    0x03E00820,
    0x04101010,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x04101010,
    0x03E00820,
    0x000007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00001E00,
    0x0001FE00,
    0x000FE000,
    0x003E0000,
    0x00700000,
    0x00600000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t blink_3[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x07F00FE0,
    0x09C81390,
    0x09C81390,
    0x07F00FE0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x007FFE00,
    0x007FFE00,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t angry[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x07000070,
    0x01C003C0,
    0x00200600,
    0x00100800,
    0x03C813C0,
    0x04242420,
    0x08100810,
    0x09C81390,
    0x09481290,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x000FF000,
    0x00300C00,
    0x00400200,
    0x00800100,
    0x007FFE00,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t shock[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x07F00FE0,
    0x08081010,
    0x10042008,
    0x03E007C0,
    0x04100820,
    0x09C81390,
    0x12242448,
    0x12242448,
    0x12242448,
    0x12242448,
    0x12242448,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x0007E000,
    0x00081000,
    0x00100800,
    0x00100800,
    0x00100800,
    0x00100800,
    0x00100800,
    0x00081000,
    0x0007E000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t blink_4[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x0FF81FF0,
    0x0FF81FF0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00C00300,
    0x00A00500,
    0x00DFFB00,
    0x00600600,
    0x00381C00,
    0x000FF000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t blink_2[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x00000000,
    0x00000000,
    0x00000000,
    0x03E007C0,
    0x04100820,
    0x09C81390,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x007FFE00,
    0x007FFE00,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t wide[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x07F00FE0,
    0x08081010,
    0x10042008,
    0x03E007C0,
    0x04100820,
    0x09C81390,
    0x12242448,
    0x12242448,
    0x12242448,
    0x12242448,
    0x12242448,
    0x09C81390,
    0x04100820,
    0x03E007C0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x001FF800,
    0x00200400,
    0x00400200,
    0x00400200,
    0x00400200,
    0x00200400,
    0x001FF800,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

const uint32_t bluetooth[32] PROGMEM = {
    0x001FF800,
    0x003FFC00,
    0x007FFE00,
    0x00FEFF00,
    0x01FE7F80,
    0x03FE3FC0,
    0x07FE1FE0,
    0x0FFE0FF0,
    0x0FFE67F0,
    0x0FCE73F0,
    0x0F8E71F0,
    0x0F8663F0,
    0x0FE007F0,
    0x0FF00FF0,
    0x0FF00FF0,
    0x0FF81FF0,
    0x0FF81FF0,
    0x0FF00FF0,
    0x0FF00FF0,
    0x0FE007F0,
    0x0F8663F0,
    0x0F8E71F0,
    0x0FCE73F0,
    0x0FFE67F0,
    0x0FFE0FF0,
    0x07FE1FE0,
    0x03FE3FC0,
    0x01FE7F80,
    0x00FEFF00,
    0x007FFE00,
    0x003FFC00,
    0x001FF800
};

/*
const uint32_t bluetooth_2[32] PROGMEM = {
    0x001FF800,
    0x00200400,
    0x00400200,
    0x00810100,
    0x01018080,
    0x0201C040,
    0x0401E020,
    0x0801F010,
    0x08019810,
    0x08318C10,
    0x08718E10,
    0x08799E10,
    0x081FF810,
    0x080FF010,
    0x080FF010,
    0x0807E010,
    0x0807E010,
    0x080FF010,
    0x080FF010,
    0x081FF810,
    0x08799E10,
    0x08718E10,
    0x08318C10,
    0x08019810,
    0x0801F010,
    0x0401E020,
    0x0201C040,
    0x01018080,
    0x00810100,
    0x00200400,
    0x001FF800
};*/

const uint32_t logo[32] PROGMEM = {
    0x00307C00,
    0x00E1FF00,
    0x01C7FF80,
    0x038FFFC0,
    0x079FFFE0,
    0x0F1FFFF0,
    0x1F3FFFF8,
    0x1E7F81F8,
    0x3E7F00FC,
    0x3C7E3C7C,
    0x3CFC3E7C,
    0x7CF81E3E,
    0x7CF81F3E,
    0x7CF81F3E,
    0x78F81F1E,
    0x78F81F1E,
    0x78F81F1E,
    0x78F81F1E,
    0x7CF81F3E,
    0x7CF81F3E,
    0x7C781F3E,
    0x3E7C3F3C,
    0x3E3C7E3C,
    0x3F00FE7C,
    0x1F81FE78,
    0x1FFFFCF8,
    0x0FFFF8F0,
    0x07FFF9E0,
    0x03FFF1C0,
    0x01FFE380,
    0x00FF8700,
    0x003E0C00
};

const uint32_t app[32] PROGMEM = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x1FDFE7F0,
    0x10529410,
    0x175665D0,
    0x174155D0,
    0x174F55D0,
    0x10559410,
    0x1FD557F0,
    0x001B1000,
    0x0752CE70,
    0x1B836220,
    0x07CC58B0,
    0x029EB3B0,
    0x087D0DC0,
    0x13826240,
    0x135FECF0,
    0x128DA200,
    0x107D3FD0,
    0x00159140,
    0x1FC355B0,
    0x10495100,
    0x1757FF60,
    0x175BE7D0,
    0x175AA810,
    0x10495390,
    0x1FCC07F0,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

// -------------------- PIXEL MAPPING --------------------
static inline void setPixel32(uint8_t x, uint8_t y, bool on)
{
  if (x > 31 || y > 31) return;

  uint8_t xx = FLIP_X ? (31 - x) : x;
  uint8_t yy = FLIP_Y ? (31 - y) : y;

  uint8_t panel = yy / 8;        // which 8-row panel (0 bottom)
  uint8_t row   = yy % 8;        // row inside panel
  uint16_t col  = xx + (uint16_t)panel * 32;

  mx.setPoint(row, col, on);
}

// Read one bit from PROGMEM bitmap
static inline bool getBitFromBitmap(const uint32_t* bmp, uint8_t x, uint8_t y)
{
  uint32_t r = pgm_read_dword(&bmp[y]);      // row y
  return (r >> (31 - x)) & 0x01;             // MSB is x=0
}

// -------------------- DRAW FULL FRAME FROM PROGMEM --------------------
void renderBitmap(const uint32_t* bmp){
  mx.clear();
  for (uint8_t y = 0; y < 32; y++)
  {
    for (uint8_t x = 0; x < 32; x++)
    {
      setPixel32(x, y, getBitFromBitmap(bmp, x, y));
    }
  }
  mx.update();
}

void renderFrame(uint8_t i){
  if (i == 0) renderBitmap(happy);
  else if (i == 1) renderBitmap(normal);
  else if (i == 2) renderBitmap(sad);
  else if (i == 3) renderBitmap(angry);
  else if (i == 4) renderBitmap(confused);
  else if (i == 5) renderBitmap(shock);
  else if (i == 6) renderBitmap(wide);
  else if (i == 7) renderBitmap(look_left);
  else if (i == 8) renderBitmap(look_right);
  else if (i == 9) renderBitmap(look_up);
  else if (i == 10) renderBitmap(look_down);
  else if (i == 11) renderBitmap(blink_2);
  else if (i == 12) renderBitmap(blink_3);
  else if (i == 13) renderBitmap(blink_4);
}

// -------------------- SETUP --------------------
void setup(){

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 5);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  bt.begin(9600);
  Serial.begin(9600);
  randomSeed(analogRead(0));


  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  renderBitmap(logo);
  delay(5000);

  renderBitmap(app);
  delay(5000);

  // renderFrame(0);
  // delay(2000);
  // if (digitalRead(statePin)) {
  // renderFrame(0);
  // } else {
  //   renderBitmap(bluetooth);
  // }
  
}

// -------------------- LOOP ----------- ---------
unsigned long lastActivityTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 8000; // Default first blink at 8s
bool isIdle = false;

int blink_duration = 0;


void side_eye_Animation() {

  unsigned long eye = millis();
  renderFrame(1);
  if (millis()-eye>=500){
  renderFrame(7);}
  if (millis()-eye>=1000){
  renderFrame(8);}
  if (millis()-eye>=1500){
  renderFrame(1);}
}

void drive(int a, int b, int c, int d) {

  digitalWrite(M1A, a);
  digitalWrite(M1B, b);
  digitalWrite(M2A, c);
  digitalWrite(M2B, d);
}


// Function that waits without blocking the Car Commands
void waitAndCheckBT(unsigned long waitTime) {
  unsigned long startWait = millis();
  while (millis() - startWait < waitTime) {
    if (bt.available()) {
      char cmd = bt.read();
      // Only process CAR commands here so the car stays responsive
      if (cmd == 'F') { drive(HIGH, LOW, HIGH, LOW); }
      else if (cmd == 'B') { drive(LOW, HIGH, LOW, HIGH); }
      else if (cmd == 'L') { drive(LOW, HIGH, HIGH, LOW); }
      else if (cmd == 'R') { drive(HIGH, LOW, LOW, HIGH); }
      else if (cmd == 'S') { drive(LOW, LOW, LOW, LOW); }
    }
  }
}

void blinkAnimation(){
    renderFrame(0); waitAndCheckBT(150);
    renderFrame(13); waitAndCheckBT(150);
    renderFrame(0); waitAndCheckBT(150);
}

void playRandomAnimation() {
  int choice = random(0, 2);
  if (choice == 0) {
    renderFrame(0); waitAndCheckBT(150);
    renderFrame(13); waitAndCheckBT(150);
    renderFrame(0); waitAndCheckBT(150);
  } else {
    renderFrame(7);  waitAndCheckBT(500);
    renderFrame(8);  waitAndCheckBT(500);
    renderFrame(1);
  }
}

void loop() {
  if (digitalRead(statePin)) {
    b= !b;
    if (c){
      renderBitmap(happy);
      Serial.println("Bluetooth Connected - Showing Happy Face");
      c=false;
    }
    if (bt.available()) {
      m = bt.read();
      Serial.print("                                Received Command: ");
      Serial.println(m);

      if ((m=='X')||(m=='x')){
        md= !md;
        Serial.print("                                Mode Toggle (md): ");
        Serial.println(md);
      }
        
      if ((m=='V')||(m=='v')){
        lr= !lr;
        Serial.print("                                Direction Toggle (lr): ");
        Serial.println(lr);
      }
      
      // 1. CAR CONTROLS (Instant)
      if (lr==true){
        if (md==true){
          if (m == 'F') { Serial.print("Forward | md=1 lr=1"); drive(HIGH, LOW, HIGH, LOW); }
          else if (m == 'B') { Serial.print("Backward | md=1 lr=1"); drive(LOW, HIGH, LOW, HIGH); }
          else if (m == 'L') { Serial.print("Left | md=1 lr=1"); drive(LOW, HIGH, HIGH, LOW); }
          else if (m == 'R') { Serial.print("Right | md=1 lr=1"); drive(HIGH, LOW, LOW, HIGH); }
          else if (m == 'S') { Serial.print("Stop | md=1 lr=1"); drive(LOW, LOW, LOW, LOW); }
        }
        else{
          if (m == 'F') { Serial.print("Forward | md=0 lr=1"); drive(HIGH, LOW, HIGH, LOW); }
          else if (m == 'B') { Serial.print("Backward Alt | md=0 lr=1"); drive(HIGH, HIGH, HIGH, HIGH); }
          else if (m == 'L') { Serial.print("Left Alt | md=0 lr=1"); drive(HIGH, HIGH, HIGH, LOW); }
          else if (m == 'R') { Serial.print("Right Alt | md=0 lr=1"); drive(HIGH, LOW, HIGH, HIGH); }
          else if (m == 'S') { Serial.print("Stop | md=0 lr=1"); drive(LOW, LOW, LOW, LOW); }
        }
      }
      else{
        if (md==true){
          if (m == 'F') { Serial.print("Forward | md=1 lr=0"); drive(HIGH, LOW, HIGH, LOW); }
          else if (m == 'B') { Serial.print("Backward | md=1 lr=0"); drive(LOW, HIGH, LOW, HIGH); }
          else if (m == 'R') { Serial.print("Right | md=1 lr=0"); drive(LOW, HIGH, HIGH, LOW); }
          else if (m == 'L') { Serial.print("Left | md=1 lr=0"); drive(HIGH, LOW, LOW, HIGH); }
          else if (m == 'S') { Serial.print("Stop | md=1 lr=0"); drive(LOW, LOW, LOW, LOW); }
        }
        else{
          if (m == 'F') { Serial.print("Forward | md=0 lr=0"); drive(HIGH, LOW, HIGH, LOW); }
          else if (m == 'B') { Serial.print("Backward Alt | md=0 lr=0"); drive(HIGH, HIGH, HIGH, HIGH); }
          else if (m == 'R') { Serial.print("Right Alt | md=0 lr=0"); drive(HIGH, HIGH, HIGH, LOW); }
          else if (m == 'L') { Serial.print("Left Alt | md=0 lr=0"); drive(HIGH, LOW, HIGH, HIGH); }
          else if (m == 'S') { Serial.print("Stop | md=0 lr=0"); drive(LOW, LOW, LOW, LOW); }
        }
      }

      // 2. MANUAL FRAME CONTROLS
      if (m == 'W' || m == 'w') {
        f++;
        Serial.print("                                Frame Increased: ");
        Serial.println(f);
        if(f > 10) {f=0;} renderFrame(f);
      }
      else if (m == 'U' || m == 'u') {
        f--;
        Serial.print("                                Frame Decreased: ");
        Serial.println(f);
        if(f < 0) {f=10;} renderFrame(f);
      }

      if (f == 11) {
        Serial.println("                                Blink Animation Triggered");
        blinkAnimation();
        f=0;
      }
    }
    // delay(10);

    // 3. IDLE & BLINK LOGIC
    unsigned long currentMillis = millis();
    nextBlinkInterval = random(5000, 8000);
    if (f == 1) {
      if (currentMillis - lastBlinkTime >= nextBlinkInterval) {
        Serial.println("                                Playing Random Animation");
        playRandomAnimation();
        lastBlinkTime = millis();
        nextBlinkInterval = random(5000, 12001);
      }
      // 60s timeout to frame 1
      if (currentMillis - lastActivityTime >= 60000) {
        Serial.println("                                Timeout → Reset to Frame 1");
        f = 1; renderFrame(1);
        lastActivityTime = millis();
      }
    }
  }
  else {
    if (b==true){
      renderBitmap(bluetooth);
      Serial.println("                                Bluetooth Disconnected - Showing Icon");
      b= !b;
      c= true;
    }
  }
}