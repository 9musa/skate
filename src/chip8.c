#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "display.h"
#include "chip8.h"


uint8_t mem[4096]; // starting address is 0x200
uint8_t V[16]; // general purpose registers
uint16_t PC; // program counter stores address
uint16_t I; // index register
uint8_t disp[WIDTH * HEIGHT]; // display
uint16_t stck[16]; // stack for 16 nested subroutine calls
uint8_t SP; // stack pointer
uint8_t keypad[16];
int keyMap[16] = {
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
  KEY_X,    // 0
  KEY_C,    // B
  KEY_FOUR, // C
  KEY_R,    // D
  KEY_F,    // E
  KEY_V     // F
};
uint8_t dTimer; // delay timer
uint8_t sTimer; // sound timer
uint16_t opcode; // opcode
AppState state = STATE_MENU;

void initChip8(void) {
  memset(mem, 0, sizeof(mem));
  memset(V, 0, sizeof(V));
  I = 0;
  memset(stck, 0, sizeof(stck));
  dTimer = 0;
  sTimer = 0;
  PC = 0x200;
  SP = 0;
  memset(disp, 0, sizeof(disp));
}
void loadROM(const char *path) {
  initChip8();
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    return;
  }
  fread(&mem[0x200], 1, 4096 - 0x200, f);
  fclose(f);
}
void chip8Cycle(void) {
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
      if (NN == 0xE0) {
        memset(disp, 0, sizeof(disp));
      }
      else if (NN == 0xEE) {
        SP -= 1;
        PC = stck[SP];
      }
      break;
    
    case (0x1):
      PC = NNN;
      break;
    
    case (0x2):
      // HANDLE STACK OVERFLOW
      stck[SP] = PC;
      SP += 1;
      PC = NNN;
      break;
    
    case (0x3):
      if (V[X] == NN) {
        PC += 2;
      }
      break;
    
    case (0x4):
      if (V[X] != NN) {
        PC += 2;
      }
      break;
    
    case (0x5):
      // ADD CHECK FOR LAST NIBBLE
      if (V[X] == V[Y]) {
        PC += 2;
      }
      break;

    case (0x6):
      V[X] = NN;
      break;
    
    case (0x7):
      V[X] += NN;
      break;

    case (0x8):
      switch (N) {
        case (0x0): // set equal
          V[X] = V[Y];
          break;
        case (0x1): // bitwise OR
          V[X] |= V[Y];
          break;
        case (0x2): // bitwise AND
          V[X] &= V[Y];
          break;
        case (0x3): // bitwise XOR
          V[X] ^= V[Y];
          break;
        case (0x4): { // VF = 1 on carry
          uint16_t sum = V[X] + V[Y];
          V[X] = sum;
          if (sum > 0xFF) {
            V[0xF] = 1;
          }
          else {
            V[0xF] = 0;
          }
          break;
        }
        case (0x5): { // VF = 0 on borrow
          uint8_t vx = V[X];
          uint8_t vy = V[Y];
          uint8_t diff = vx - vy;
          uint8_t vf = (vx >= vy);
          V[X] = diff;
          V[0xF] = vf;
          break;
        }
        case (0x6): // VF = old LSB
          uint8_t lsb = V[Y] & (0x01);
          V[X] = V[Y] >> 1;
          V[0xF] = lsb;
          break;
        case (0x7): { // VF = 0 on borrow
          uint8_t vx = V[X];
          uint8_t vy = V[Y];
          uint8_t diff = vy - vx;
          uint8_t vf = (vy >= vx);
          V[X] = diff;
          V[0xF] = vf;
          break;
        }
        case (0xE): // VF = old MSB
          uint8_t msb = (V[Y] & (0x80)) >> 7;
          V[X] <<= 1;
          V[0xF] = msb;
          break;
      }
      break;

    case (0x9):
      // ADD CHECK FOR LAST NIBBLE
      if (V[X] != V[Y]) {
        PC += 2;
      }
      break;
    
    case (0xA):
      I = NNN;
      break;
    
    case (0xB):
      PC = (NNN + V[0]);
      break;

    case (0xC): {
      uint8_t R = rand() % 256;
      V[X] = R & NN;
      break;
    } // keep R contained
    
    case (0xD): {
      V[0xF] = 0;
      int x = V[X];
      int y = V[Y];
      for (int row = 0; row < N; row++) {
        //int correctedY = (y + row) % height; // wrap around for SCHIP
        uint8_t spriteByte = mem[I + row]; // extract drawing byte
        for (int col = 0; col < 8; col++) {
          //int correctedX = (x + col) % WIDTH; // wraps around
          int bit = (spriteByte >> (7 - col)) & 1; // isolate one bit
          if (!bit) continue; // skip if 0
          //int index = (correctedY) * WIDTH + (correctedX); // wrap around for SCHIP
          if ((x + col) >= WIDTH || (y + row) >= HEIGHT) continue; // bounds check
          int index = (y + row) * WIDTH + (x + col);
          if (disp[index] == 1 && bit == 1) {
            V[0xF] = 1; // collision flag
          }
          disp[index] ^= bit; // XOR
        }
      }
      break;
    }

    case (0xE):
      switch (NN) {
        case (0x9E):
          if (keypad[V[X]]) {
            PC += 2;
          }
          break;
        
        case (0xA1):
          if (!keypad[V[X]]) {
            PC += 2;
          }
          break;
      }

    case (0xF):
      switch (NN) {
        case (0x07):
          V[X] = dTimer;
          break;
        case (0x0A):
          int keyPressed = 0;
          for (int i = 0; i < 16; i++) {
            if (keypad[i]) {
              V[X] = i;
              keyPressed = 1;
              break;
            }
          }
          if (!keyPressed) {
            PC -= 2;
          }
          break;
        case (0x15):
          dTimer = V[X];
          break;
        case (0x18):
          sTimer = V[X];
          break;
        case (0x1E):
          I += V[X];
          break;
        case (0x29):
          I = 0x50 + V[X] * 5;
          break;
        case (0x33):
          mem[I] = V[X] / 100;
          mem[I + 1] = (V[X] / 10) % 10;
          mem[I + 2] = V[X] % 10;
          break;
        case (0x55):
          for (int i = 0; i <= X; i++) {
            mem[I + i] = V[i];
          }
          break;
        case (0x65):
          for (int i = 0; i <= X; i++) {
            V[i] = mem[I + i];
          }
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
  SetExitKey(KEY_NULL);
  scanFolder();
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      if (state == STATE_RUNNING) {
        state = STATE_MENU;
      }
      else if (state == STATE_MENU) {
        break;
      }
    }
    switch (state) {
      case STATE_MENU:
        updateMenu();
        drawMenu();
        break;
      case STATE_RUNNING:
        chip8Cycle();
        drawDisplay();
        break;
    }
  }
  closeDisplay();
  return 0;
}