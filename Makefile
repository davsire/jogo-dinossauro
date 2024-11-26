TARGET = jogo_dino
CC = g++
OBJ = jogo_dino.c
PARAMS = -lncurses -lpthread

jogo: $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(PARAMS)

clean:
	rm $(TARGET)
