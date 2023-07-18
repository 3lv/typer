#ifndef SCREEN_H
#define SCREEN_H
#include <iostream>
#include <string.h>
#include <vector>

class Coords;
class Buffer;
class Window;
class Screen;

class Coords { /* {{{ */
private:
public:
	unsigned int i;
	unsigned int j;
	Coords() {
		i = 0;
		j = 0;
	}
	Coords(int _i, int _j) {
		i = _i;
		j = _j;
	}
	Coords(unsigned int _i, unsigned int _j) {
		i = _i;
		j = _j;
	}
}; /* }}} */

class Buffer {
private:
public:
	Screen *screen;
	Window *window;
	std::string text;
	std::vector<std::string>lines;
	Buffer();
	Buffer(Window *window);
	void update();
};

class Window {
private:
public:
	Screen *screen; // parent screen
	Buffer *buffer; // child buffer
	Coords __coords;
	unsigned int __lines;
	unsigned int __cols;
	Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols);
	void update(Coords coords, unsigned int lines, unsigned int cols);
};

class Screen {
private:
public:
	unsigned int __lines;
	unsigned int __cols;
	Coords __coords;
	Coords __text_end_coords;
	std::vector<Window*> windows;
	Screen();
	const unsigned int lines();
	const unsigned int cols();
	void __update ();
	unsigned int create_win(Coords coords, unsigned int lines, unsigned int cols);
	Coords coords();
	void save_coords();
	void restore_coords();
	void move(const unsigned int i, const unsigned int j);
	bool in_screen(Coords coords);
	void move(Coords __coords);
	void rect(const unsigned int I, const unsigned int J, const unsigned int height, const unsigned int width);
	void draw_win(const unsigned int winnr);
	void draw();
	void render();
};

#endif // SCREEN_H
// vim:fdm=marker:
