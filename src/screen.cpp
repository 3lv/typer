#include <iostream>
#include <fstream>
#include <signal.h>
#include "../include/screen.h"
#ifdef _WIN32
# include <windows.h>
#endif

namespace _Screen {/*{{{*/
	std::ofstream LOG("debug.log");
	inline void resize_handler(int) {
		Screen::screen.resize();
		//Screen::resize();
	}
}
/*}}}*/

/*{{{ color and _option*/
// TODO: move these to screen.h without messing linker ld
namespace color {
	color::Hl_group hl_NonText = "\033[0;34m";
	color::Hl_group hl_EndOfBuffer = "\033[0;34m";
}
namespace _option {
	Iopt_t tabwidth = 8;
	Sopt_t tab = std::string(tabwidth, ' ');
}/*}}}*/

/*{{{ display_char and print_strlen*/
std::string display_chr(char chr) {
	if(chr > ' ') {
		return std::string(1, chr);
	}
	if(chr == ' ') {
		return " ";
	}
	if(chr == 8) { // ^I
		return _option::tab;
	}
	if(chr == 0) {
		return "^@";
	}
	if(chr < 0) {
		return "";
	}
	if(chr <= 26) {
		char ctrlchr = 'A' + chr - 1;
		return (std::string)"^" + ctrlchr;
	}
	if(chr == 27) {
		return "^]";
	}
	if(chr == 28) {
		return "^\\";
	}
	if(chr == 29) {
		return "^]";
	}
	if(chr == 30) {
		return "^^";
	}
	if(chr == 31) {
		return "^_";
	}
	return "";
}
size_t print_strlen(std::string line) {
	size_t n = line.size();
	size_t plen = 0;
	for(size_t i = 0; i < n; ++i) {
		plen += display_chr(line[i]).size();
	}
	return plen;
}/*}}}*/

namespace Term { /*{{{*/
#ifdef _WIN32
	DWORD old_dwMode;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	void vt_proc() {
		GetConsoleMode(hConsole, &old_dwMode);
		DWORD dwMode = old_dwMode;
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hConsole, dwMode);
	}
	void vt_proc_reset() {
		SetConsoleMode(hConsole, old_dwMode);
	}
	void clear_screen() {
		system("cls");
	}
	void change_buffer() {
		//not useful
		HANDLE hNewBuffer = CreateConsoleScreenBuffer(
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				CONSOLE_TEXTMODE_BUFFER,
				NULL);
		if(hNewBuffer == INVALID_HANDLE_VALUE) {
			return;
		}
		SetConsoleActiveScreenBuffer(hNewBuffer);
	}
	void buffer_reset() {
		SetConsoleActiveScreenBuffer(hConsole);
	}
	void setup_term() {
		clear_screen();
		// cannot use change_buffer cuz output printed to the default buffer
		//change_buffer();
		vt_proc();
		//term_raw();
	}
	void reset_term() {
		vt_proc_reset();
		//term_raw_reset();
	}
#else
	void setup_term() {
		system("stty raw -echo");
		// move to the alternate terminal buffer
		std::cout << "\033[?1049h" << std::flush;
	}
	void reset_term() {
		std::cout << std::flush;
		// restore the default buffer
		std::cout << "\033[?1049l" << std::flush;
		// restore normal modes
		system("stty cooked echo");
		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
#endif
}/*}}}*/

/*{{{ Coords*/
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
} /*}}}*/

/*{{{ text parsing function*/
Buffer::vtext_t split(std::string text, char split_at = '\n') {
	Buffer::vtext_t vtext;
	std::string line;
	for(char& c : text) {
		if(c == split_at) {
			//line += '\n'; // optional?
			vtext.push_back(line);
			line = "";
		} else {
			line += c;
		}
	}
	if(line != "") {
		vtext.push_back(line);
	}
	return vtext;
}
color::ctext_t cstring_parse(std::string cstring) {
	std::string active_color = color::RESET_COLOR;
	std::string text = "";
	color::cline_t cline;
	std::string _color = "";
	// multiple colors one after another
	bool color_secv = true;
	for(size_t i = 0; i < cstring.size(); ++i) {
		if(cstring[i] == '\033') {
			for(; cstring[i] != 'm' && i < cstring.size(); ++i) {
				_color += cstring[i];
			}
			if(cstring[i] != 'm') {
				// error
				continue;
			}
			_color += "m";
			if(color_secv == false) {
				active_color = "";
			}
			active_color += _color;
			color_secv = true;
			continue;
		}
		color_secv = false;
		text += cstring[i];
		cline.push_back(color::ccell_t(active_color, color::RESET_COLOR));
	}
	return color::ctext_t(text, cline);
}
Buffer::vctext_t vcstring_parse(std::vector<std::string> vcstring) {
	Buffer::vtext_t vtext;
	color::vcline_t vcline;
	color::ctext_t ctext;
	for(size_t i = 0; i < vcstring.size(); ++i) {
		ctext = cstring_parse(vcstring[i]);
		vtext.push_back(ctext.first);
		vcline.push_back(ctext.second);
	}
	return Buffer::vctext_t(vtext, vcline);
}
/*}}}*/

/*{{{ Buffer*/
Buffer::Buffer() {
	window = NULL;
	screen = NULL;
}
Buffer::Buffer(Window *window) {
	Buffer();
	this->window = window;
	screen = window->screen;
}
void Buffer::change_content(vtext_t vtext) {
	_buffer = vtext;
}
void Buffer::change_color(color::vcline_t vcline) {
	_buffer = vcline;
}
size_t Buffer::size() {
	return _buffer.size();
}
Buffer::line_t Buffer::operator[](size_t idx) {
	return _buffer[idx];
}
Buffer::buf_t* Buffer::operator+(size_t idx) {
	return _buffer + idx;
}
/*}}}*/

/*{{{ Window*/
Window::Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols): __coords(coords), viewport(this) {
	this->screen = screen;
	buffer = new Buffer(this);
	__lines = lines;
	__cols = cols;
	viewport.first_line = 0;
	buf_cursor_coords = Coords();
};
void Window::update(Coords coords, unsigned int lines, unsigned int cols) {
	this->__coords = coords;
	this->__lines = lines;
	this->__cols = cols;
	//?redraw window
}
void Window::buf_text(Buffer::vtext_t vtext) {
	// TODO: viewport.first_line should be a pointer to first buffer line
	viewport.first_line = 0;
	// TODO: change cursor position to first char of first line
	//
	buffer->change_content(vtext);
	screen->draw_win(this);
}
void Window::buf_color(color::vcline_t vcline) {
	buffer->change_color(vcline);
	screen->draw_win(this);
}
void Window::buf_text(std::string text) {
	buf_text(split(text));
}
// change buffer's text with color new_text
void Window::buf_ctext(std::string ctext) {
	Buffer::vctext_t vctext = vcstring_parse(split(ctext));
	//viewport.first_line = 0;
	//buf_cursor_coords = Coords();
	buffer->change_content(vctext.first);
	buffer->change_color(vctext.second);
	screen->draw_win(this);
	// TODO: by default scroll all the way to the top
}
/*}}}*/

/*{{{ Cursor*/
Cursor::Cursor(Screen *screen) : stored_coords{Coords(0,0)} {
	this->screen = screen;
	window = NULL;
}
Coords Cursor::_coords() {
	unsigned int i, j;
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(Term::hConsole, &csbi);
	COORD pos = csbi.dwCursorPosition;
	i = pos.Y;
	j = pos.X;
#else
	std::cout << "\033[6n" << std::flush;
	scanf("\033[%d;%dR", &i, &j);
	-- i;
	-- j;
#endif
	return Coords(i, j);
}
void Cursor::_move(const unsigned int i, const unsigned int j) {
	// 1 based index for \033[<L>;<C>H ansi escape code
	std::cout << "\033["
	<< std::to_string(i + 1) + ";" + std::to_string(j + 1)
	<< "H" << std::flush;
}
void Cursor::_move(Coords coords) {
	_move(coords.i, coords.j);
}
void Cursor::_save_coords() {
	stored_coords[0] = _coords();
}
void Cursor::_restore_coords() {
	_move(stored_coords[0]);
}/*}}}*/

/*{{{ Screen */
Screen Screen::screen;
Screen::Screen() {
	this->cursor = new Cursor(this);
}
void Screen::init() {
	Term::setup_term();
	this->__update();
#ifdef _WIN32
	//TODO
#else
	signal(SIGWINCH, _Screen::resize_handler);
#endif
	__is_init = true;
}
Screen::~Screen() {
	if(__is_init) {
		Term::reset_term();
	}
}
Window* Screen::create_win(Coords coords, unsigned int lines, unsigned int cols) {
	Window *new_win = new Window(this, coords, lines, cols);
	windows.push_back(new_win);
	new_win->winnr = windows.size() - 1;
	return new_win;
}
// just updates the size, doesn't return
void Screen::__update_size() {
	cursor->_save_coords();
	cursor->_move(9998,9998);
	Coords lc = cursor->_coords();
	this->__lines = lc.i;
	this->__cols = lc.j;
	cursor->_restore_coords();
}
void Screen::__update() {
	this->__update_size();
}
void Screen::draw_win(Window *window) {
	cursor->_save_coords();
	Coords c_coords = window->__coords;
	cursor->_move(c_coords);
	Buffer buf = *(window->buffer);
	int n = buf.size();
	n = std::min(n, 20);
	//int n = buf->size();
	for(int i = 0; i < n; ++i) {
		List<cell_t> line = buf[i];
		int m = line.size();
		for(int j = 0; j < m; ++j) {
			// o(n) complexity
			std::cout << (cell_t)line[j];
		}
		c_coords.i++;
		cursor->_move(c_coords);
	}
	std::cout << color::RESET_COLOR << std::flush;
	cursor->_restore_coords();
}
void Screen::resize() {
#ifdef _WIN32
	system("cls");
#else
	//std::cout << "\003[2J";
	system("clear");
#endif
	// TODO
}/*}}}*/

// vi:fdm=marker
