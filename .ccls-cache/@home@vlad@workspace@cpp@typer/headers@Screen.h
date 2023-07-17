#ifndef SCREEN_H
#define SCREEN_H
#include <iostream>
#include <string.h>
#include <vector>

class Coords;
class Header; // tehnically buf1
class Buffer; // buf2
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
};
std::ostream& operator<<(std::ostream &os, const Coords &c) {
	return os << "(" << c.i << ";" << c.j << ")";
} /* }}} */

class Header {
private:
	std::string text;
	Coords __coords; // (1;1)
	Coords __end_coords;
	Screen *screen;
public:
	Header(Screen *sc);
	void change_text(const std::string new_text);
};

class Buffer {
	private:
	public:
		unsigned int __lines;
		unsigned int __cols;
		Coords __coords;
		std::string text;
		std::vector<std::string>lines;
		Screen *screen;
		Buffer(Screen *screen);
		void update();
};

class Screen {
private:
public:
	unsigned int __lines;
	unsigned int __cols;
	Coords __coords;
	Coords __text_end_coords;
	Header *header;
	Buffer *buffer;
	// TODO vector<Buffer_ptr> buffers;
	Screen();
	const unsigned int lines();
	const unsigned int cols();
	void __update ();
	Coords coords();
	void save_coords();
	void restore_coords();
	void move(const unsigned int i, const unsigned int j);
	bool in_screen(Coords coords);
	void move(Coords __coords);
	void rect(const unsigned int I, const unsigned int J, const unsigned int height, const unsigned int width);
	void draw_win(Buffer *buffer);
	void draw();
	void render();
};

#endif // SCREEN_H
// vim:fdm=marker:
