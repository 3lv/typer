#include <iostream>
#include <fstream>
#include <string.h>
#include <signal.h>
#include "../include/screen.h"

#ifdef _WIN32
# include <windows.h>
#endif

// move these to screen.h without messing linker ld
namespace color {
	color::Hl_group hl_NonText = "\033[0;34m";
	color::Hl_group hl_EndOfBuffer = "\033[0;34m";
}

// not well implemented
size_t printable_strlen(std::string line) {
	bool in_control = false;
	size_t n = line.size();
	size_t plen = 0;
	for(int i = 0; i < n; ++i) {
		if(line[i] == '\033' && line[i + 1] == '[') {
			in_control = true;
			++i;
		} else if(in_control) {
			if(line[i] == 'm') {
				in_control = false;
			}
		}
		else {
			plen++;
		}
	}
	return plen;
}

namespace Term {
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
		buffer_reset();
		//term_raw_reset();
	}
#else
	void setup_term() {
		system("stty raw -echo");
		// move to the alternate terminal buffer
		std::cout << "\033[?1049h" << std::flush;
	}
	void reset_term() {
		// restore the default buffer
		std::cout << std::flush;
		std::cout << "\033[?1049l" << std::flush;
		// restore normal modes
		system("stty cooked echo");
		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
#endif
}

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

Window::Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols): __coords(coords), viewport(this) {
	this->screen = screen;
	this->buffer = new Buffer(this);
	__lines = lines;
	__cols = cols;
	this->viewport.first_line = 0;
};
void Window::update(Coords coords, unsigned int lines, unsigned int cols) {
	this->__coords = coords;
	this->__lines = lines;
	this->__cols = cols;
	this->update_lines();
	//?redraw window
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
				} else if(printable_strlen(lines[lines.size() - 1]) + printable_strlen(word) <= this->__cols) {
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
	// by default scroll all the way to the top
	viewport.first_line = 0;
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
void Cursor::move(const unsigned int i, const unsigned int j) {
	// 1 based index for \033[<L>;<C>H ansi escape code
	std::cout << "\033["
	<< std::to_string(i + 1) + ";" + std::to_string(j + 1)
	<< "H" << std::flush;
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

namespace _Screen {
	inline void resize_handler(int signal_nul) {
		Screen::screen.resize();
		//Screen::resize();
	}
}
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
void Screen::printl(Window *win, std::string line) {
	std::cout << line;
	// TODO: string::size() is not accurate for escape chars, implement one
	size_t n = line.size();
	const int spaces = (int)win->__cols - printable_strlen(line);
	for(int i = 0; i < spaces; ++ i) {
		std::cout << " ";
	}
}

void Screen::draw_win(Window *window) {
	cursor->save_coords();
	Coords c_coords = window->__coords;
	cursor->move(c_coords);
	int first_visible_line = window->viewport.first_line;
	int last_visible_line = first_visible_line + window->__lines - 1; // this can be after the last window->line[]
	bool first_line_visible = false, last_line_visible = false;
	if(first_visible_line == 0) {
		first_line_visible = true;
	}
	if(last_visible_line >= window->lines.size() - 1) {
		last_line_visible = true;
	}
	std::cout << color::RESET_COLOR;
	for(int line_idx = first_visible_line; line_idx <= last_visible_line; ++ line_idx) {
		cursor->move(c_coords); // in_screen(c_coords) should always by true
		if(line_idx == first_visible_line) {
			if(!first_line_visible) {
				printl(window, color::hl_NonText + "@@@");
				std::cout << color::RESET_COLOR;
				continue;
			}
		}
		if(line_idx == last_visible_line) {
			if(!last_line_visible && window->lines.size() >= 2) {
				printl(window, color::hl_NonText + "@@@");
				continue;
			}
		}
		if(line_idx < window->lines.size()) {
			printl(window, window->lines[line_idx]);
			//std::cout << printable_strlen(window->lines[line_idx]);
		} else {
			printl(window, color::hl_EndOfBuffer + "~");
		}
		// TODO: clear what was previously displayed/ fill line with spaces function
		++ c_coords.i;
	}
	std::cout << color::RESET_COLOR;
	cursor->restore_coords();
}
void Screen::draw_win(const unsigned int winnr) {
	this->draw_win(this->windows[winnr]);
}

void Screen::resize() {
#ifdef _WIN32
	system("cls");
#else
	//std::cout << "\003[2J";
	system("clear");
#endif
	// PROVIZORIU
	this->__update();
	int winn = windows.size();
	int si = this->__lines;
	int sj = this->__cols;
	int game_height = (si + 1) * 0.6;
	int game_width = (sj + 1) * 0.8;
	//game_width = std::min(game_width, (int)text_length);
	game_width = std::min(game_width, 100);
	int starti = (si - game_height) / 2;
	int startj = (sj - game_width) / 2;
	screen.windows[0]->update( Coords(starti, startj),
			2,
			game_width
			);
	// screen->__lines numarul de linii
	screen.windows[1]->update( Coords(starti + 2, startj),
			game_height - 2,
			game_width
			);
	//DEBUG
	if(winn > 0) {
		this->windows[0]->buf_text(
				std::to_string(this->windows[1]->lines.size())
				);
	}
	for(int winnr = 0; winnr < winn; ++ winnr) {
		this->draw_win(winnr);
	}
	if(winn >= 2) {
		cursor->window = windows[1];
		cursor->move(windows[1]->__coords);
		/* // TODO
		// moves the cursor the windows' buffer last position
		cursor->move(windows[1]);
		*/
	}
}

// vim:fdm=marker:
