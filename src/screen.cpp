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

Header::Header(Screen *sc): __coords(0,0) {
		text = "";
		this->screen = sc;
}
void Header::change_text(const std::string new_text) {
	screen->save_coords();
        screen->move(this->__coords);
	screen->move(0,0);
        // TODO use a global function for CCODE transformations
	system("stty echo");
        std::cout << "\033[0m"; // reset highlighting
        std::cout << new_text;
	// TODO fix this for, text.size() isn't accurate, use coords instead
	// OR add a clear header text function that should be ran before printing
        for (int i = new_text.size() + 1; i <= this->text.size(); ++i) {
                std::cout << " ";
        }
        this->text = new_text;
        std::cout << "\033[0m";
	std::cout << std::flush;
	system("stty -echo");
        screen->restore_coords();
}

Buffer::Buffer(Screen *sc) : __coords(1,0) {
	__lines = 0;
	__cols = 0;
	text = "";
	this->screen = sc;
}
void Buffer::update() {
	screen->__update();
	this->__cols = screen->__cols;
	std::string word;
	lines.clear();
	for(int i = 0; i < text.size(); ++ i) {
		if(text[i] != ' ') {
			word += text[i];
		} else {
			if(lines[lines.size() - 1].size() + word.size() <= this->__cols) {
				lines[lines.size() - 1] += word;
			} else {
				lines.push_back(word);
			}
		}
	}
	if(lines[lines.size() - 1].size() + word.size() <= this->__cols) {
		lines[lines.size() - 1] += word;
	} else {
		lines.push_back(word);
	}
}


Screen::Screen() {
	__lines = 0;
	__cols = 0;
	header = new Header(this);
	buffer = new Buffer(this);
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
	save_coords();
}
Coords Screen::coords() {
	unsigned int i, j;
	std::string opts = exec("stty -g");
	system("stty -echo");
	std::cout << "\033[6n" << std::flush;
	//fflush(stdout);
	scanf("\033[%d;%dR", &i, &j);
	system(("stty " + opts).c_str());
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
	const std::string cmd = "tput cup "
		+ std::to_string(i) + " "
		+ std::to_string(j);
	system(cmd.c_str());
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
	//this->move(I, J);
	for(int i = 1; i < width - 1; ++ i) {
		std::cout << "\b\b" << "#";
	}
	std::cout << "\b\b" << "X"; // bot left corner
	for(int i = 1; i < height - 1; ++ i) {
		std::cout << "\033[A\b" << "#";
	}
} /* }}} */
void Screen::draw_win(Buffer *buffer) {
	buffer->update();
	Coords c_coords = buffer->__coords;
	for(int line = 0; line < buffer->__lines; ++ line) {
		if(this->in_screen(c_coords))
			this->move(c_coords);
		std::cout << buffer->lines[line];
		++ c_coords.i;
	}
}
void Screen::draw() {
	system("clear");
	this->__update();
	//rect(0, 0, this->lines(), this->cols());
	this->header->change_text("Screen resized");
	this->draw_win(this->buffer);
}

