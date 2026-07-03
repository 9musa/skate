#include <stdint.h>


uint8_t mem[4096]; // starting address is 0x200
uint8_t V[16]; // general purpose registers
uint16_t PC; // program counter stores address
uint16_t I; // index register
uint8_t disp[64 * 32]; // display
uint16_t stck[16]; // 16 nested subroutine calls
uint8_t SP; // stack pointer
uint8_t dTimer; // delay timer
uint8_t sTimer; // sound timer
uint16_t opcode;


int main(void) {
  PC = 0x200;
  SP = 0;
  while (1) {
    opcode = (mem[PC] << 8 | mem[PC + 1]);
    PC += 2;
    uint8_t T = (opcode & 0xF000) >> 12; // first nibble
    uint8_t X = (opcode & 0x0F00) >> 8; // second nibble
    uint8_t Y = (opcode & 0x00F0) >> 4; // third nibble
    uint8_t N = (opcode & 0x000F); // fourth nibble
    uint8_t NN = (opcode & 0x00FF); // last byte
    uint16_t NNN = (opcode & 0x0FFF); // last 12 bits

    switch (T)
    {
    case (0x0):
      // NEST LATER
      break;
    
    case (0x1):
      PC = NNN;
      break;
    
    case (0x2):
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
      // NEST LATER
      break;

    case (0x9):
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
}