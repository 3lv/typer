#include <iostream>
#include <fstream>
#include <string.h>
#include <signal.h>
#include "../include/screen.h"

Coords::Coords() {
	this->i = 0;
	this->j = 0;
}
Coords::Coords(unsigned int _i, unsigned int _j) {
	this->i = _i;
	this->j = _j;
}
const Coords Coords::operator+(const Coords &coords) {
	return Coords(this->i + coords.i, this->j + coords.j);
}

Buffer::Buffer(): cursor_coords(0,0) {
	text = "";
	this->window = NULL;
	this->screen = NULL;
}
Buffer::Buffer(Window *window): cursor_coords(0,0) {
	text = "";
	this->window = window;
	this->screen = window->screen;
}
void Buffer::update() {
	this->lines.clear();
	int text_lenght = this->text.size();
	std::string line = "";
	// TODO: FIXME: might have problems with 2width chars
	for(int i = 0; i < text_lenght; ++ i) {
		if(text[i] == '\n') {
			lines.push_back(line);
			line = "";
		} else {
			line += text[i];
		}
	}
	if(line.size() != 0) {
		lines.push_back(line);
	}
}
void Buffer::change_text(std::string new_text) {
	this->text = new_text;
	this->update();
	this->window->update_lines();
	// TODO this call s ugly
	this->screen->draw_win(this->window);
	//this->window->redraw();
}

Window::Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols): __coords(coords) {
	this->screen = screen;
	this->buffer = new Buffer(this);
	__lines = lines;
	__cols = cols;
};
void Window::update(Coords coords, unsigned int lines, unsigned int cols) {
	this->__coords = coords;
	this->__lines = lines;
	this->__cols = cols;
	this->buffer->update();
	//redraw window
}
void Window::update_lines() {
	bool break_word = false;
	this->lines.clear();
	// TODO: USE POINTER INSTEAD
	std::vector<std::string>buf_lines = this->buffer->lines;
	std::string word = "";
	for(int li = 0; li < buf_lines.size(); ++ li) {
		for(int idx = 0; idx < buf_lines[li].size(); ++ idx) {
			char cur_ch = buf_lines[li][idx];
			if(cur_ch != ' ') {
				word += cur_ch;
			} else {
				word += " ";
				// TODO: FIXME: if 1 word longer than line
				if(lines.size() == 0) {
					lines.push_back(word);
				} else if(lines[lines.size() - 1].size() + word.size() <= this->__cols) {
					lines[lines.size() - 1] += word;
				} else {
					lines.push_back(word);
				}
				word = "";
			}
		}
		if(word.size() != 0) {
			if(lines.size() == 0) {
				lines.push_back(word);
			} else if(lines[lines.size() - 1].size() + word.size() <= this->__cols) {
				lines[lines.size() - 1] += word;
			} else {
				lines.push_back(word);
			}
			word = "";
		}
	}
	this->__dplen.clear();
	__dplen.push_back(lines[0].size());
	for(int i = 1; i < lines.size(); ++ i) {
		__dplen.push_back(__dplen[i - 1] + lines[i].size());
	}
}
void Window::buf_text(std::string new_text) {
	this->buffer->change_text(new_text);
}
Coords Window::nth_char(unsigned int pos) {
	int ac = 0;
	int pas = 1<<30;
	int n = __dplen.size();
	while(pas) {
		if(ac + pas < n && __dplen[ac + pas] < pos) {
			ac += pas;
		}
		pas >>= 1;
	}
	return Coords(ac, pos - __dplen[ac]);
}

Cursor::Cursor(Screen *screen) : stored_coords{Coords(0,0)}, __coords(0,0) {
	this->screen = screen;
	window = NULL;
}
Coords Cursor::coords() {
	unsigned int i, j;
	std::cout << "\033[6n" << std::flush;
	scanf("\033[%d;%dR", &i, &j);
	-- i;
	-- j;
	return Coords(i, j);
}
void Cursor::move(const unsigned int i, const unsigned int j) {
	// 1 based index for \033[<L>;<C>H ansi escape code
	std::cout << "\033["
	+ std::to_string(i + 1) + ";" + std::to_string(j + 1)
	+ "H" << std::flush;
	this->__coords = Coords(i, j);
}
void Cursor::move(Coords __coords) {
	move(__coords.i, __coords.j);
}
void Cursor::move(Window *win) {
	if(this->window == win) {
		return;
	}
	this->window = win;
	this->move(win->__coords + win->buffer->cursor_coords);
}
void Cursor::move_inwin(Direction dir, bool wrap) {
	if(dir == Direction::none) {
		return;
	}
	if(dir == Direction::right) {
	}
}
void Cursor::save_coords() {
	this->stored_coords[0] = this->coords();
}
void Cursor::restore_coords() {
	move(this->stored_coords[0]);
}
void cazan() {
	std::cout << "resized";
}

Screen::Screen() {
	system("stty raw -echo");
	// move to the alternate terminal buffer
	std::cout << "\033[?1049h" << std::flush;
	this->cursor = new Cursor(this);
	this->__update();
	//signal(SIGWINCH, Screen::resize);
}
Screen::~Screen() {
	// restore to the default buffer
	std::cout << std::flush;
	std::cout << "\033[?1049l" << std::flush;
	// restore normal modes
	system("stty cooked echo");
}
unsigned int Screen::create_win(Coords coords, unsigned int lines, unsigned int cols) {
	Window *new_win = new Window(this, coords, lines, cols);
	windows.push_back(new_win);
	return new_win->winnr = windows.size() - 1;
}
// just updates the size, doesn't return
void Screen::__update_size() {
	cursor->save_coords();
	cursor->move(9998,9998);
	Coords lc = cursor->coords();
	this->__lines = lc.i;
	this->__cols = lc.j;
	cursor->restore_coords();
}
void Screen::__update() {
	this->__update_size();
}
bool Screen::in_screen(Coords coords) {
	return (0 <= coords.i && coords.i < this->__lines) &&
		(0 <= coords.j && coords.j < this->__cols);
}
void Screen::rect(const unsigned int I, const unsigned int J, /* {{{ */
		const unsigned int height, const unsigned int width) {
	/*
	 *	OUTDATED
	 *      1 ------> 2
	 *      ^         |
	 *      |         |
	 *      |         |
	 *      |         v
	 *      4 <------ 3
	 */
	/*
	 *      1 ------> 2
	 *      3         |
	 *      |         |
	 *      |         |
	 *      v         v
	 *      4 ----->5 3
	 *
	 */
	cursor->save_coords();
	cursor->move(I, J);
	std::cout << "┌"; // top left corner
	for(int i = 1; i < width - 1; ++ i) {
		std::cout << "─";
	}
	std::cout << "┐"; // top right corner
	for(int i = 1; i < height - 1; ++ i) {
		std::cout << "\n\b" << "│";
	}
	std::cout << "\n\b";
	std::cout << "┘"; // bot right corner
	cursor->move(I, J + 1);
	for(int i = 1; i < height - 1; ++ i) {
		std::cout << "\n\b" << "│";
	}
	std::cout << "\n\b" << "└"; // bot left corner
	for(int i = 1; i < width - 1; ++ i) {
		std::cout << "─";
	}
	cursor->restore_coords();
} /* }}} */
void Screen::draw_win(Window *window) {
	cursor->save_coords();
	Coords c_coords = window->__coords;
	cursor->move(c_coords);
	int starting_line = 0;
	int ending_line = window->lines.size() - 1;
	if(ending_line - starting_line + 1 > window->__lines) {
		ending_line = window->__lines + starting_line - 1;
	}
	std::cout << color::RESET_COLOR;
	for(int line = starting_line; line <= ending_line; ++ line) {
		if(this->in_screen(c_coords)) {
			cursor->move(c_coords);
		}
		std::cout << window->lines[line];
		const int spaces = window->__cols - window->lines[line].size();
		// TODO: clear what was previously displayed
		for(int i = 0; i < spaces; ++ i) {
			std::cout << " ";
		}
		// std::cout << "spaces to clear previous text";
		++ c_coords.i;
	}
	std::cout << color::RESET_COLOR;
	cursor->restore_coords();
}
void Screen::draw_win(const unsigned int winnr) {
	this->draw_win(this->windows[winnr]);
}

void Screen::resize(int signum) {
	// PROVIZORIU
	this->__update();
	int winn = windows.size();
	for(int winnr = 0; winnr < winn; ++ winnr) {
		//windows[winnr]->update();
		// TEMP
		if(winnr == 0) {
			windows[winnr]->update(
					windows[0]->__coords,
					windows[0]->__lines,
					this->__cols - windows[0]->__coords.j
					);
		} else if (winnr == 1) {
			windows[winnr]->update(
					windows[1]->__coords,
					this->__lines - windows[1]->__coords.i,
					this->__cols - windows[1]->__coords.j
					);
		}
	}
	for(int winnr = 0; winnr < winn; ++ winnr) {
		this->draw_win(winnr);
	}
	if(winn >= 2) {
		cursor->window = windows[1];
		cursor->move(windows[1]->__coords);
	}
}

// vim:fdm=marker:
