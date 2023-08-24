#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <vector>
#include "list.h"

class Coords;
class Buffer_t;
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

namespace color {
	typedef const std::string ANSIcode;
	typedef std::string Hl_group;
	/* Consts {{{ */
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
	/* }}} */
	struct ccell_t {
		typedef std::string color_t;
		color_t before;
		color_t after;
		ccell_t() {
			before = "";
			after = "";
		}
		ccell_t(color_t bef, color_t aft) {
			before = bef;
			after = aft;
		}
	};
	typedef std::vector<ccell_t> cline_t;
	typedef std::vector<cline_t> vcline_t;
	typedef std::pair<std::string, cline_t>ctext_t;
}

namespace _option {
	typedef std::string Sopt_t;
	typedef int Iopt_t;
}

struct cell_t {
	unsigned char byte;
	color::ccell_t color;
	cell_t() {
		byte = 0;
	}
	cell_t(char ch) {
		byte = ch;
	}
	operator char() const {
		return byte;
	}
	void operator=(char ch) {
		byte = ch;
	}
	void operator=(color::ccell_t col) {
		color = col;
	}
	friend std::ostream& operator<<(std::ostream &os, cell_t &cell) {
		return os << cell.color.before << cell.byte << cell.color.after;
	}
};


class Buffer {
public:
	typedef List<cell_t> line_t;
	//typedef List<line_t> buf_t;
	typedef List<List<cell_t>> buf_t;
	typedef std::vector<cell_t> vline_t;
	typedef std::vector<vline_t> vbuf_t;
	typedef std::vector<std::string> vtext_t;
	typedef std::pair<vtext_t, color::vcline_t> vctext_t;
private:
	buf_t _buffer;
public:
	Screen *screen;
	// Parent window
	Window *window;
	// (relative to the Parent window.__coords)
	Buffer();
	Buffer(Window *window);
	void change_content(vtext_t vtext);
	void change_color(color::vcline_t vcline);
	size_t size();
	line_t operator[](size_t idx);
	buf_t* operator+(size_t idx);
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
	// has to be kept up to date
	Coords buf_cursor_coords;
	// vector that holds the partial sum of the length of the lines
	unsigned int __lines;
	unsigned int __cols;
	struct Viewport {
		Window *window;
		Buffer::line_t* first_line;
		Viewport(Window *win) {
			this->window = win;
			first_line = 0;
		}
	} viewport;
	Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols);
	void update(Coords coords, unsigned int lines, unsigned int cols);
	void buf_text(Buffer::vtext_t vtext);
	void buf_text(std::string next);
	void buf_color(color::vcline_t vcline);
	void buf_ctext(std::string ctext);
	// scroll by n lines (+/-)
	//void scroll(int num_lines);
};

class Cursor {
private:
public:
	static Cursor cursor;
	Screen *screen;
	Window *window;
	Coords stored_coords[10];
	Cursor(Screen *screen);
	Coords _coords();
	void _move(unsigned int i, unsigned int j);
	void _move(Coords coords);
	// like <space> in vim
	// TODO: function implementation
	void ____move_right();
	void _save_coords();
	void _restore_coords();
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
	void rect(const unsigned int I, const unsigned int J, const unsigned int height, const unsigned int width);
	// Print Color Line
	void draw_win(Window *win);
	void resize();
};

#endif // SCREEN_H

// vi:fdm=marker
