#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"


uint8_t mem[4096]; // starting address is 0x200
uint8_t V[16]; // general purpose registers
uint16_t PC; // program counter stores address
uint16_t I; // index register
uint8_t disp[64 * 32]; // display
uint16_t stck[16]; // stack for 16 nested subroutine calls
uint8_t SP; // stack pointer
uint8_t dTimer; // delay timer
uint8_t sTimer; // sound timer
uint16_t opcode; // opcode

void initChip8(void) {
  PC = 0x200;
  SP = 0;
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
          uint8_t diff = V[X] - V[Y];
          if (V[X] >= V[Y]) {
            V[0xF] = 1;
          }
          else {
            V[0xF] = 0;
          }
          V[X] = diff;
          break;
        }
        case (0x6): // VF = old LSB
          V[0xF] = V[X] & (0x01);
          V[X] >>= 1;
          break;
        case (0x7): { // VF = 0 on borrow
          uint8_t diff = V[Y] - V[X];
          if (V[Y] >= V[X]) {
            V[0xF] = 1;
          }
          else {
            V[0xF] = 0;
          }
          V[X] = diff;
          break;
        }
        case (0xE): // VF = old MSB
          V[0xF] = V[X] & (0x80);
          V[0xF] >>= 7;
          V[X] <<= 1;
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
    
    case (0xD):
      // SET UP GRAPHICS
      break;

    case (0xE):
      // SET UP KEYS
      break;

    case (0xF):
      // NEST LATER
      break;
      
    default:
      break;
  }
}


int main(void) {
  initChip8();
  initDisplay();
  while (!WindowShouldClose()) {
    chip8Cycle();
    drawDisplay();
  }
  closeDisplay();
  return 0;
}