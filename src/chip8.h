#define WIDTH 128
#define HEIGHT 64
#include <stdint.h>
#include <stdbool.h>


void loadROM(const char *path);


extern uint8_t mem[4096];
extern uint8_t V[16];
extern uint16_t PC;
extern uint16_t I;
extern uint16_t stck[16];
extern uint8_t SP;
extern uint8_t dTimer;
extern uint8_t sTimer;
extern uint16_t opcode;
extern uint8_t disp[WIDTH * HEIGHT];
extern bool highResMode;
typedef enum { STATE_MENU, STATE_RUNNING } AppState;
extern AppState state;