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


int main(void) {
  //main
}