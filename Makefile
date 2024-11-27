TARGET = jogo_dino
CC = g++
OBJS = jogo_dino.o missel.o
PARAMS = -lncurses -lpthread

jogo: $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(PARAMS)

jogo_dino.o: src/jogo_dino.c src/jogo_dino.h src/missel.h
	$(CC) -c src/jogo_dino.c $(PARAMS)

missel.o: src/missel.c src/missel.h
	$(CC) -c src/missel.c $(PARAMS)

clean:
	rm $(TARGET) $(OBJS)
