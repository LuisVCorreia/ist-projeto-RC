# Makefile for the AS application

CC = g++

TARGET = AS

SOURCES = app.cpp clientUDP.cpp

all:
	$(CC) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.o
