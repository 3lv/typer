#ifndef SCREEN_H
#define SCREEN_H
#include <iostream>
#include <string.h>
#include <vector>
#include <map>

class Coords;
class Buffer;
class Window;
class Screen;
class Cursor;

class Coords {
private:
public:
	unsigned int i;
	unsigned int j;
	Coords();
	Coords(unsigned int _i, unsigned int _j);
	const Coords operator+(const Coords &coords);
};

	/* Consts {{{ */
namespace color {
	typedef std::string ANSIcode;
	const ANSIcode RESET_COLOR = "\033[0m";
	const ANSIcode FG_BLACK = "\033[30m";
	const ANSIcode FG_RED = "\033[31m";
	const ANSIcode FG_GREEN = "\033[32m";
	const ANSIcode FG_YELLOW = "\033[33m";
	const ANSIcode FG_BLUE = "\033[34m";
	const ANSIcode FG_MAGENTA = "\033[35m";
	const ANSIcode FG_CYAN = "\033[36m";
	const ANSIcode FG_WHITE = "\033[37m";
	const ANSIcode FG_BLACK_L = "\033[30;1m";
	const ANSIcode FG_RED_L = "\033[31;1m";
	const ANSIcode FG_GREEN_L = "\033[32;1m";
	const ANSIcode FG_YELLOW_L = "\033[33;1m";
	const ANSIcode FG_BLUE_L = "\033[34;1m";
	const ANSIcode FG_MAGENTA_L = "\033[35;1m";
	const ANSIcode FG_CYAN_L = "\033[36;1m";
	const ANSIcode FG_WHITE_L = "\033[37;1m";
	const ANSIcode BG_BLACK = "\033[40m";
	const ANSIcode BG_RED = "\033[41m";
	const ANSIcode BG_GREEN = "\033[42m";
	const ANSIcode BG_YELLOW = "\033[44m";
	const ANSIcode BG_BLUE = "\033[44m";
	const ANSIcode BG_MAGENTA = "\033[45m";
	const ANSIcode BG_CYAN = "\033[46m";
	const ANSIcode BG_WHITE = "\033[47m";
	const ANSIcode BG_BLACK_L = "\033[40;1m";
	const ANSIcode BG_RED_L = "\033[41;1m";
	const ANSIcode BG_GREEN_L = "\033[42;1m";
	const ANSIcode BG_YELLOW_L = "\033[43;1m";
	const ANSIcode BG_BLUE_L = "\033[42;4m";
	const ANSIcode BG_MAGENTA_L = "\033[45;1m";
	const ANSIcode BG_CYAN_L = "\033[46;1m";
	const ANSIcode BG_WHITE_L = "\033[47;1m";
}
/* }}} */

class Buffer {
private:
public:
	Screen *screen;
	// Parent window
	Window *window;
	// (relative to the Parent window.__coords)
	Coords cursor_coords;
	std::string text;
	std::vector<std::string>lines;
	Buffer();
	Buffer(Window *window);
	void update();
	void change_text(std::string new_text);
};

class Window {
private:
public:
	// Parent screen
	Screen *screen;
	// Child buffer
	Buffer *buffer;
	unsigned int winnr;
	Coords __coords;
	std::vector<std::string>lines;
	// vector that holds the partial sum of the length of the lines
	std::vector<int> __dplen;
	unsigned int __lines;
	unsigned int __cols;
	Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols);
	void last_filled_line();
	void last_col();
	void update(Coords coords, unsigned int lines, unsigned int cols);
	// Update lines to fit the window
	void update_lines();
	void buf_text(std::string new_text);
	// finds the coords of element on specific position
	Coords nth_char(unsigned int pos);
};

class Cursor {
private:
public:
	enum class Direction{
		none, up, down, right, left
	};
	Screen *screen;
	Window *window;
	Coords stored_coords[10];
	// lazy loaded coords (might not be accurate)
	Coords __coords;
	// calculated coords
	Coords buffer_coords;
	Cursor(Screen *screen);
	Coords coords();
	void move(unsigned int i, unsigned int j);
	void move(Coords coords);
	void move(Window *win);
	// TODO: function implementation
	void move_inwin(Direction dir, bool wrap);
	void save_coords();
	void restore_coords();
};

class Screen {
private:
public:
	std::vector<Window*> windows;
	Cursor *cursor;
	unsigned int __lines;
	unsigned int __cols;
	Coords __coords;
	Screen();
	~Screen();
	void __update_size();
	void __update();
	unsigned int create_win(Coords coords, unsigned int lines, unsigned int cols);
	bool in_screen(Coords coords);
	void rect(const unsigned int I, const unsigned int J, const unsigned int height, const unsigned int width);
	void draw_win(Window *win);
	void draw_win(const unsigned int winnr);
	void resize(int signum);
};

#endif // SCREEN_H
// vim:fdm=marker:
