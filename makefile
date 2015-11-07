CC = gcc
CFLAGS = -pedantic -Wall -Werror -std=gnu99 -O3 -o
COMPILEC = $(CC) $(CFLAGS)

CPPC = g++
CPPFLAGS = -pedantic -Wall -std=c++11 -O3 -o
COMPILECPP = $(CPPC) $(CPPFLAGS)
LIBS = -larmadillo -lSDL -pthread

OBJECTS = ARGBot.o serial.o ARGBotRunner.o

all: $(OBJECTS) ARGBot

serial.o: serial.c serial.h
	$(COMPILEC) $@ -c $<

ARGBot.o: ARGBot.cpp ARGBot.h
	$(COMPILECPP) $@ -c $<

ARGBotRunner.o: ARGBotRunner.cpp
	$(COMPILECPP) $@ -c $<

ARGBot: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

clean:
	rm -rfv *.o ARGBot