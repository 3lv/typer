MAJOR_VERSION := 1
MINOR_VERSION := 0
VERSION := v${MAJOR_VERSION}.$(MINOR_VERSION)

CC = gcc
CXX = g++
DEFINES =
LINK = g++
LFLAGS = -Wl,-O1
CXXFLAGS = -pipe -O2 -Wall -W $(DEFINES)
INCPATH = -I../

SOURCES = ../src/main.cpp \
	  ../src/screen.cpp
OBJECTS = main.o \
	  screen.o
TARGET  = typer

first: all

all: Makefile $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS)

main.o: 	../src/main.cpp \
		../include/screen.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o ../src/main.cpp

screen.o:	../src/screen.cpp \
		../include/screen.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o screen.o ../src/screen.cpp


install: $(TARGET)
	@echo "${VERSION}"
	@echo "Installing documents.."
	@mkdir -p ~/documents/typer
	@cp -r ../langs ~/documents/typer/langs
	@echo "Done!"
	@echo "Installing binary.."
	@cp typer ~/bin/typer
	@echo "Done!"
	@echo "Everything done!"

clean:
	@rm -f "typer" $(OBJECTS)
	@echo "Cleaned!"
