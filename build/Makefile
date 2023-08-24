MAJOR_VERSION := 1
MINOR_VERSION := 0
VERSION := v$(MAJOR_VERSION).$(MINOR_VERSION)

CXX = g++
WINCXX = x86_64-w64-mingw32-g++
INCPATH = -I../
# CXXFLAGS = -Wall -Werror -Wextra -W
CXXFLAGS = -Wall -W

# debug build settings
DBGCXXFLAGS = -g -DDEBUG

# release build settings
RELCXXFLAGS = -O3 -static -DNDEBUG

SOURCES = ../src/main.cpp \
	  ../src/list.cpp \
	  ../src/screen.cpp
OBJECTS = main.o \
	  list.o \
	  screen.o

TARGET  = typer
WINTARGET = typer.exe

.PHONY: all clean debug release

all: release

win: CXX = $(WINCXX)
win: TARGET = $(WINTARGET)
win: release

debug: CXXFLAGS += $(DBGCXXFLAGS)
debug: $(TARGET)

release: CXXFLAGS += $(RELCXXFLAGS)
release: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	@ #if [ ${TERM} -eq "dumb" ]; then echo "dumb"; else echo "smart"; fi
	@echo "##### $(TARGET) built! #####"

main.o:		../src/main.cpp \
		../include/screen.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o ../src/main.cpp

screen.o:	../src/screen.cpp \
		../include/list.h \
		../include/screen.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o screen.o ../src/screen.cpp

install: $(TARGET)
	@echo "Installing ${TARGET} ${VERSION}"
	@mkdir -p ~/documents/typer
	@cp -r ../langs ~/documents/typer/langs
	@cp typer ~/bin/typer
	@echo "Done!"

clean:
	@rm -f *.o $(TARGET)
	@echo "Cleaned!"
