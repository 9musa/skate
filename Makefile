CC = gcc
SRC = src/chip8.c src/display.c
OUT = C-8.exe
PROJECT_RESOURCE_FILE = resource.rc
RES_OBJ = resource.o

all:
	windres $(PROJECT_RESOURCE_FILE) -o $(RES_OBJ)
	$(CC) $(SRC) $(RES_OBJ) -Iraylib/include -Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows -o $(OUT)
clean:
	del $(OUT) $(RES_OBJ)