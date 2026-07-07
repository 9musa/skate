#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "display.h"
#include "chip8.h"


uint8_t mem[4096]; // starting address is 0x200
uint8_t V[16]; // general purpose registers
uint16_t PC; // program counter stores address
uint16_t I; // index register
uint8_t disp[WIDTH * HEIGHT]; // display
bool highResMode; // high-res low-res toggle
uint16_t stck[16]; // stack for 16 nested subroutine calls
uint8_t SP; // stack pointer
uint8_t keypad[16];
int keyMap[16] = {
  KEY_X,    // 0
  KEY_ONE,  // 1
  KEY_TWO,  // 2
  KEY_THREE,// 3
  KEY_Q,    // 4
  KEY_W,    // 5
  KEY_E,    // 6
  KEY_A,    // 7
  KEY_S,    // 8
  KEY_D,    // 9
  KEY_Z,    // A
  KEY_C,    // B
  KEY_FOUR, // C
  KEY_R,    // D
  KEY_F,    // E
  KEY_V     // F
};
unsigned char fontSet[240] = {
  // low-res
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80, // F

  // high-res
  0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, // 0
  0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, // 1
  0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, // 2
  0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C, // 3
  0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, // 4
  0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C, // 5
  0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, // 6
  0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, // 7
  0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, // 8
  0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C  // 9
};
uint8_t dTimer; // delay timer
uint8_t sTimer; // sound timer
Sound beep; // beep sound
uint16_t opcode; // opcode
AppState state = STATE_MENU; // initialise with menu


void initChip8(void) {
  // clear everything for fresh start
  memset(mem, 0, sizeof(mem));
  memset(V, 0, sizeof(V));
  I = 0;
  memset(stck, 0, sizeof(stck));
  dTimer = 0;
  sTimer = 0;
  PC = 0x200;
  SP = 0;
  highResMode = false;
  memset(disp, 0, sizeof(disp));
  memcpy(mem, fontSet, sizeof(fontSet)); // load fonts to memory
}

void loadROM(const char *path) {
  initChip8();
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    return;
  }
  fread(&mem[0x200], 1, 4096 - 0x200, f); // load ROM at 0x200 according to spec
  fclose(f);
}

void insCycle(void) {
  opcode = (mem[PC] << 8 | mem[PC + 1]);
  PC += 2;
  uint8_t T = (opcode & 0xF000) >> 12; // first nibble
  uint8_t X = (opcode & 0x0F00) >> 8; // second nibble
  uint8_t Y = (opcode & 0x00F0) >> 4; // third nibble
  uint8_t N = (opcode & 0x000F); // fourth nibble
  uint8_t NN = (opcode & 0x00FF); // last byte
  uint16_t NNN = (opcode & 0x0FFF); // last 12 bits

  switch (T) {
    case (0x0):
      switch (NN) {
        case (0xE0): { // CLR SCREEN
          memset(disp, 0, sizeof(disp));
          break;
        }
        case (0xEE): { // RTRN SUBROUTINE
          if (SP == 0) {
            // STACK UNDERFLOW
            closeDisplay();
            // potentially add windows err box
          } else {
          SP -= 1;
          PC = stck[SP];
          }
          break;
        }
        case (0xFE): { // OFF HI-RES CLR SCREEN
          highResMode = false;
          memset(disp, 0, sizeof(disp));
          break;
        }
        case (0xFF): { // ON HI-RES CLR SCREEN
          highResMode = true;
          memset(disp, 0, sizeof(disp));
          break;
        }
        case (0xFB): { // SCROLL RIGHT
          int width = highResMode ? 128 : 64;
          int height = highResMode ? 64 : 32;
          int amount = 4;
          for (int y = 0; y < height; y++) {
            memmove(&disp[y * 128 + amount], &disp[y * 128], width - amount);
            memset(&disp[y * 128], 0, amount);
          }
          break;
        }
        case (0xFC): { // SCROLL LEFT
          int width = highResMode ? 128 : 64;
          int height = highResMode ? 64 : 32;
          int amount = 4;
          for (int y = 0; y < height; y++) {
            memmove(&disp[y * 128], &disp[y * 128 + amount], width - amount);
            memset(&disp[y * 128 + (width - amount)], 0, amount);
          }
          break;
        }

        case (0xFD): // EXIT
         closeDisplay();
         break;
        
        default: // SCROLL DOWN
          if ((NN & 0xF0) == 0xC0) {
            int numPixels = (opcode & 0xF);
            int width = highResMode ? 128 : 64;
            int height = highResMode ? 64 : 32;
            for (int y = height - 1; y >= numPixels; y--) {
              memcpy(&disp[y * 128], &disp[(y - numPixels) * 128], width);
            }
            for (int y = 0; y < numPixels; y++) {
              memset(&disp[y * 128], 0, width);
            }
          } else {
            // INVALID OPCODE
            break;
          }
          break;
      }
      break;
    
    case (0x1): // JUMP ADDRESS
      PC = NNN;
      break;
    
    case (0x2): // CALL ADDRESS
      // STACK OVERFLOW
      if (SP >= 16) {
        // OVERFLOW
        closeDisplay();
        // potentially add windows error box later
      } else {
        stck[SP] = PC;
        SP += 1;
        PC = NNN;
      }
      break;
    
    case (0x3): // SKIP IF EQUAL
      if (V[X] == NN) {
        PC += 2;
      }
      break;
    
    case (0x4): // SKIP IF NOT EQUAL
      if (V[X] != NN) {
        PC += 2;
      }
      break;
    
    case (0x5): // SKIP IF Vx EQUAL Vy
      // CHECK FOR LAST NIBBLE
      if (N != 0) {
        // INVALID OPCODE
        break;
      }
      if (V[X] == V[Y]) {
        PC += 2;
      }
      break;

    case (0x6): // SET Vx
      V[X] = NN;
      break;
    
    case (0x7): // INCREMENT Vx
      V[X] += NN;
      break;

    case (0x8): // ALU
      switch (N) {
        case (0x0): // SET EQUAL
          V[X] = V[Y];
          break;
        case (0x1): // BITWISE OR
          V[X] |= V[Y];
          break;
        case (0x2): // BITWISE AND
          V[X] &= V[Y];
          break;
        case (0x3): // BITWISE XOR
          V[X] ^= V[Y];
          break;
        case (0x4): { // Vf EQUAL 1 on carry
          uint16_t sum = V[X] + V[Y];
          V[X] = sum;
          if (sum > 0xFF) {
            V[0xF] = 1;
          } else {
            V[0xF] = 0;
          }
          break;
        }
        case (0x5): { // Vf EQUAL 0 on borrow
          uint8_t vx = V[X];
          uint8_t vy = V[Y];
          uint8_t diff = vx - vy;
          uint8_t vf = (vx >= vy);
          V[X] = diff;
          V[0xF] = vf;
          break;
        }
        case (0x6): // Vf EQUAL old LSB
          uint8_t lsb = V[X] & (0x01);
          V[X] >>= 1;
          V[0xF] = lsb;
          break;
        case (0x7): { // Vf EQUAL 0 on borrow
          uint8_t vx = V[X];
          uint8_t vy = V[Y];
          uint8_t diff = vy - vx;
          uint8_t vf = (vy >= vx);
          V[X] = diff;
          V[0xF] = vf;
          break;
        }
        case (0xE): // Vf EQUAL old MSB
          uint8_t msb = (V[Y] & (0x80)) >> 7;
          V[X] <<= 1;
          V[0xF] = msb;
          break;
      }
      break;

    case (0x9): // SKIP IF NOT EQUAL
      // CHECK FOR LAST NIBBLE
      if (N != 0) {
        // INVALID OPCODE
        break;
      }
      if (V[X] != V[Y]) {
        PC += 2;
      }
      break;
    
    case (0xA): // SET INDEX
      I = NNN;
      break;
    
    case (0xB): // JUMP WITH OFFSET
      PC = NNN + V[X];
      break;

    case (0xC): { // RANDOM
      uint8_t R = rand() % 256;
      V[X] = R & NN;
      break;
    } // keep R contained
    
    case (0xD): { // DRAW SPRITE
      V[0xF] = 0; // ini collision flag
      int currentWidth  = highResMode ? 128 : 64;
      int currentHeight = highResMode ? 64  : 32;
      int x = V[X] % currentWidth; // wrapping
      int y = V[Y] % currentHeight; // wrapping
      bool isMegaSprite = (highResMode && N == 0);
      int rows = isMegaSprite ? 16 : N;
      int cols = isMegaSprite ? 16 : 8;
      for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
          int drawX = (x + col);
          int drawY = (y + row);
          if (drawX >= currentWidth || drawY >= currentHeight) continue;
          int bit = 0;
          if (isMegaSprite) { // mega read
            int byteOffset = col / 8; // 0 for first byte, 1 for sec byte
            uint8_t spriteByte = mem[I + (row * 2) + byteOffset]; // 2 rows and skip
            bit = (spriteByte >> (7 - (col % 8))) & 1;
          } else { // standard read
            uint8_t spriteByte = mem[I + row]; // 1 row and skip
            bit = (spriteByte >> (7 - col)) & 1; // left to right
          }
          if (!bit) continue; // skip 0
          int index = drawY * 128 + drawX;
          if (disp[index] == 1 && bit == 1) {
            V[0xF] = 1; // collision
          }
          disp[index] ^= bit; // XOR mapping
        }
      }
      break;
    }

    case (0xE): // KEYPAD
      switch (NN) {
        case (0x9E):
          if (keypad[V[X]] & 0x0F) {
            PC += 2;
          }
          break;
        
        case (0xA1):
          if (!keypad[V[X] & 0x0F]) {
            PC += 2;
          }
          break;
        
        default:
          // INVALID OPCODE
          break;
      }
      break;

    case (0xF): // SYSTEM
      switch (NN) {
        case (0x07): // SET REGISTER TO DELAY
          V[X] = dTimer;
          break;
        case (0x0A): { // SET REGISTER IF KEY PRESSED AND RELEASED
          static int heldKey = -1; // tracks index across frames
          int pressedKey = -1; // holds key num
          for (int i = 0; i < 16; i++) {
            if (keypad[i]) {
              pressedKey = i;
              break;
            }
          }
          if (heldKey == -1) { // waiting for initial key press
            if (pressedKey != -1) {
              heldKey = pressedKey; // catch it
            }
            PC -= 2; // rewind
          } else { // already caught a key press in previous frame
            if (keypad[heldKey]) {
              PC -= 2; // still held down, await
            } else {
              V[X] = heldKey; // released, set register
              heldKey = -1; // reinitialise
            }
          }
          break;
        }
        case (0x15): // SET DELAY
          dTimer = V[X];
          break;
        case (0x18): // SET BUZZER
          sTimer = V[X];
          break;
        case (0x1E): // INCREMENT INDEX
          I += V[X];
          break;
        // MASK BECAUSE FONT CONTAIN A MAXIMUM OF 10 ELEMENTS
        case (0x29): // POINT INDEX TO LOW-RES FONT
          I = (V[X] & 0x0F) * 5;
          break;
        case (0x30): // POINT INDEX TO HIGH-RES FONT
          I = 80 + ((V[X] & 0x0F) * 10);
          break;
        case (0x33): // BINARY CODE DECIMAL
          mem[I] = V[X] / 100;
          mem[I + 1] = (V[X] / 10) % 10;
          mem[I + 2] = V[X] % 10;
          break;
        case (0x55): // SAVE REGISTERS TO MEM
          for (int i = 0; i <= X; i++) {
            mem[I + i] = V[i];
          }
          break;
        case (0x65): // LOAD REGISTERS TO MEM
          for (int i = 0; i <= X; i++) {
            V[i] = mem[I + i];
          }
          break;
        default:
          // INVALID OPCODE
          break;
      }
      break;
      
    default:
      break;
  }
}


int main(void) {
  initChip8();
  initDisplay();
  InitAudioDevice();
  SetTargetFPS(60);
  SetExitKey(KEY_NULL); // removes raylib default exit key
  scanFolder();
  static float timerAccumulator = 0.0f;
  beep = LoadSound("assets/beep.wav");
  while (!WindowShouldClose()) {
    // keypad
    for (int i = 0; i < 16; i++) {
      keypad[i] = IsKeyDown(keyMap[i]);
    }

    // timers
    timerAccumulator += GetFrameTime();
    while (timerAccumulator >= 1.0f / 60.0f) {
      if (dTimer > 0) dTimer--;
      if (sTimer > 0) sTimer--;
      timerAccumulator -= 1.0f / 60.0f;
    }

    // sound
    if (sTimer > 0) {
      if (!IsSoundPlaying(beep)) {
        PlaySound(beep);
      }
    } else {
      if (IsSoundPlaying(beep)) {
        StopSound(beep);
      }
    }

    // navigation
    if (IsKeyPressed(KEY_ESCAPE)) {
      if (state == STATE_RUNNING) {
        state = STATE_MENU;
      } else if (state == STATE_MENU) {
        break;
      }
    }
    switch (state) {
      case STATE_MENU:
        updateMenu();
        drawMenu();
        break;
      case STATE_RUNNING:
        for (int i = 0; i < 10; i++) {
          insCycle();
        }
        drawDisplay();
        break;
    }

  }
  UnloadSound(beep);
  CloseAudioDevice();
  closeDisplay();
  return 0;
}