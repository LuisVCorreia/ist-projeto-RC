# Makefile for the AS application

CC = g++

TARGET = AS

SOURCES = User.cpp Client.cpp clientUDP.cpp clientTCP.cpp

all:
	$(CC) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.o
