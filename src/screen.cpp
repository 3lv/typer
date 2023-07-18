#include <iostream>
#include <string.h>
#include "../include/screen.h"

/* {{{
char* exec(const std::string command) { 
	FILE* fp;
	char* line = NULL;
	// Following initialization is equivalent to char* result = ""; and just
	// initializes result to an empty string, only it works with
	// -Werror=write-strings and is so much less clear.
	char* result = (char*) calloc(1, 1);
	size_t len = 0;
	fflush(NULL);
	fp = popen(command.c_str(), "r");
	if (fp == NULL) {
		std::cout << "Cannot execute command: " + command;
		return NULL;
	}
	while(getline(&line, &len, fp) != -1) {
		// +1 below to allow room for null terminator.
		result = (char*) realloc(result, strlen(result) + strlen(line) + 1);
		// +1 below so we copy the final null terminator.
		strncpy(result + strlen(result), line, strlen(line) + 1);
		free(line);
		line = NULL;
	}
	fflush(fp);
	if (pclose(fp) != 0) {
		perror("Cannot close stream.\n");
	}
	return result;
} }}} */

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
	std::string word = "";
	this->lines.clear();
	int text_lenght = this->text.size();
	for(int i = 0; i < text_lenght; ++ i) {
		if(text[i] != ' ') {
			word += text[i];
		} else {
			word += " ";
			if(lines.size() == 0) {
				lines.push_back(word);
			} else if(lines[lines.size() - 1].size() + word.size() <= this->window->__cols) {
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
		} else if(lines[lines.size() - 1].size() + word.size() <= this->window->__cols) {
			lines[lines.size() - 1] += word;
		} else {
			lines.push_back(word);
		}
	}
}
void Buffer::change_text(std::string new_text) {
	this->text = new_text;
	this->update();
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
void Window::buf_text(std::string new_text) {
	this->buffer->change_text(new_text);
}

Cursor::Cursor(Screen *screen) : __coords(0,0) {
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
void Cursor::save_coords() {
	this->__coords = this->coords();
}
void Cursor::restore_coords() {
	move(this->__coords);
}

Screen::Screen() {
	this->cursor = new Cursor(this);
	this->__update();
}
unsigned int Screen::create_win(Coords coords, unsigned int lines, unsigned int cols) {
	Window *new_win = new Window(this, coords, lines, cols);
	windows.push_back(new_win);
	return new_win->winnr = windows.size() - 1;
}
// just updates the size, doesn't return
void Screen::__update_size() {
	unsigned int i, j;
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
	 *       1 ------> 2
	 *       ^         |
	 *       |         |
	 *       |         |
	 *       |         v
	 *       4 <------ 3
	 *
	 */
	cursor->move(I, J);
	std::cout << "X"; // top left corner
	for(int i = 1; i < width - 1; ++ i) {
		std::cout << "#";
	}
	std::cout << "X"; // top right corner
	for(int i = 1; i < height - 1; ++ i) {
		std::cout << "\033[B" << "#";
	}
	std::cout << "\033[B";
	std::cout << "X"; // bot right corner
	std::cout << "\b" << "#";
	for(int i = 1; i < width - 1; ++ i) {
		std::cout << "\b\b" << "#";
	}
	std::cout << "\b\b" << "X"; // bot left corner
	for(int i = 1; i < height - 1; ++ i) {
		std::cout << "\033[A\b" << "#";
	}
} /* }}} */
void Screen::draw_win(Window *window) {
	cursor->save_coords();
	Buffer *buffer = window->buffer;
	Coords c_coords = window->__coords;
	cursor->move(c_coords);
	int starting_line = 0;
	int ending_line = buffer->lines.size() - 1;
	if(ending_line - starting_line + 1 > window->__lines) {
		ending_line = window->__lines + starting_line - 1;
	}
	std::cout << color::RESET_COLOR;
	for(int line = starting_line; line <= ending_line; ++ line) {
		if(this->in_screen(c_coords)) {
			cursor->move(c_coords);
		}
		std::cout << buffer->lines[line];
		++ c_coords.i;
	}
	std::cout << color::RESET_COLOR;
	cursor->restore_coords();
}
void Screen::draw_win(const unsigned int winnr) {
	this->draw_win(this->windows[winnr]);
}
void Screen::draw() {
	system("clear");
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
