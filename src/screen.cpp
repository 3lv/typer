#include <iostream>
#include <string.h>
#include "../include/screen.h"

char* exec(const std::string command) { /* {{{ */
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
} /* }}} */

Buffer::Buffer() {
	text = "";
	this->window = NULL;
	this->screen = NULL;
}
Buffer::Buffer(Window *window) {
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
	if(lines.size() == 0) {
		lines.push_back(word);
	} else if(lines[lines.size() - 1].size() + word.size() <= this->window->__cols) {
		lines[lines.size() - 1] += word;
	} else {
		lines.push_back(word);
	}
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

Screen::Screen() {
	this->__update();
	//BY DEFAULT SCREEN CREATES 2 WINDOWS
}
unsigned int Screen::create_win(Coords coords, unsigned int lines, unsigned int cols) {
	Window *new_win = new Window(this, coords, lines, cols);
	this->windows.push_back(new_win);
	return windows.size();
}
const unsigned int Screen::lines() {
	this->__lines = atoi(exec("tput lines"));
	return this->__lines;
}
const unsigned int Screen::cols() {
	this->__cols = atoi(exec("tput cols"));
	return this->__cols;
}
void Screen::__update () {
	lines();
	cols();
}
Coords Screen::coords() {
	unsigned int i, j;
	std::cout << "\033[6n" << std::flush;
	scanf("\033[%d;%dR", &i, &j);
	i --;
	j --;
	return Coords(i, j);
}
void Screen::save_coords() {
	this->__coords = this->coords();
}
void Screen::restore_coords() {
	move(this->__coords);
}
void Screen::move(const unsigned int i, const unsigned int j) {
	// 1 based index for \033[<L>;<C>H ansi escape code
	std::cout << "\033["
	+ std::to_string(i + 1) + ";" + std::to_string(j + 1)
	+ "H" << std::flush;
}
bool Screen::in_screen(Coords coords) {
	return (0 <= coords.i && coords.i < this->__lines) &&
		(0 <= coords.j && coords.j < this->__cols);
}
void Screen::move(Coords __coords) {
	move(__coords.i, __coords.j);
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
	this->move(I, J);
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
void Screen::draw_win(const unsigned int winnr) {
	this->save_coords();
	Window *window = this->windows[winnr];
	Buffer *buffer = window->buffer;
	Coords c_coords = window->__coords;
	this->move(c_coords);
	int starting_line = 0;
	int ending_line = buffer->lines.size();
	if(ending_line - starting_line + 1 > window->__lines) {
		ending_line = window->__lines + starting_line - 1;
	}
	for(int line = starting_line; line < ending_line; ++ line) {
		if(this->in_screen(c_coords))
			this->move(c_coords);
		std::cout << buffer->lines[line];
		++ c_coords.i;
	}
	this->restore_coords();
}
void Screen::draw() {
	system("clear");
	this->__update();
	//rect(0, 0, this->lines(), this->cols());
	this->windows[0]->update(
			windows[0]->__coords,
			windows[0]->__lines,
			this->__cols
			);
	this->windows[1]->update(
			windows[1]->__coords,
			this->__lines - windows[0]->__lines,
			this->__cols
			);
	for(int i_win = 0; i_win < windows.size(); ++ i_win) {
		this->draw_win(i_win);
	}
	this->move(windows[1]->__coords);
}

// vim:fdm=marker:
