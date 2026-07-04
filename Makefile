CC = gcc
SRC = src/c-8.c src/display.c
OUT = C-8.exe

all:
	$(CC) $(SRC) -Iraylib/include -Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -o $(OUT)