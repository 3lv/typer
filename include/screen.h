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
	typedef const std::string ANSIcode;
	typedef std::string Hl_group;
	ANSIcode RESET_COLOR = "\033[0m";
	ANSIcode LIGHT = "\033[1m";
	ANSIcode UNDERLINE = "\033[4m";
	ANSIcode FG_BLACK = "\033[30m";
	ANSIcode FG_RED = "\033[31m";
	ANSIcode FG_GREEN = "\033[32m";
	ANSIcode FG_YELLOW = "\033[33m";
	ANSIcode FG_BLUE = "\033[34m";
	ANSIcode FG_MAGENTA = "\033[35m";
	ANSIcode FG_CYAN = "\033[36m";
	ANSIcode FG_WHITE = "\033[37m";
	ANSIcode BG_BLACK = "\033[40m";
	ANSIcode BG_RED = "\033[41m";
	ANSIcode BG_GREEN = "\033[42m";
	ANSIcode BG_YELLOW = "\033[44m";
	ANSIcode BG_BLUE = "\033[44m";
	ANSIcode BG_MAGENTA = "\033[45m";
	ANSIcode BG_CYAN = "\033[46m";
	ANSIcode BG_WHITE = "\033[47m";
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
	struct Viewport {
		Window *window;
		unsigned int first_line;
		unsigned int _last_line;
		unsigned int last_line() {
			return first_line + window->__lines - 1;
		}
		Viewport(Window *win) {
			this->window = win;
			first_line = 0;
		}
	} viewport;
	unsigned int __v_first_line = 0;
	Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols);
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
	static Cursor cursor;
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
	Screen();
	Screen(Screen *other);
	//Screen &operator=(Screen *other);
	bool __is_init;
public:
	// TODO: make this const
	// All windows and buffers should use Screen::screen;
	static Screen screen;
	void init();
	std::vector<Window*> windows;
	Cursor *cursor;
	unsigned int __lines;
	unsigned int __cols;
	Coords __coords;
	~Screen();
	void __update_size();
	void __update();
	Window* create_win(Coords coords, unsigned int lines, unsigned int cols);
	bool in_screen(Coords coords);
	void rect(const unsigned int I, const unsigned int J, const unsigned int height, const unsigned int width);
	void printl(Window *win, std::string line);
	void draw_win(Window *win);
	void draw_win(const unsigned int winnr);
	void resize();
};

#endif // SCREEN_H
// vim:fdm=marker:
