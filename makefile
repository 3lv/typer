MAJOR_VERSION := 1
MINOR_VERSION := 0
VERSION := v${MAJOR_VERSION}.$(MINOR_VERSION)
all: executable

install: typer
	@echo "${VERSION}"
	mkdir -p ~/documents/typer
	cp -r langs ~/documents/typer/langs
	cp typer ~/bin/typer
	rm typer

executable: main.cpp
	$(CXX) main.cpp -o typer
