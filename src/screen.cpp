#include <iostream>
#include <fstream>
#include <string.h>
#include <signal.h>
#include "../include/screen.h"
#ifdef _WIN32
# include <windows.h>
#endif

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
	for(int i = 0; i < n; ++i) {
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

/*{{{ Buffer*/
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
	this->lines.clear();
	this->clines.clear();
	std::string line = "";
	color::cvector_t cline;
	for(int i = 0; i < text.size(); ++ i) {
		if(text[i] == '\n') {
			// TODO: add \n to the line but don't display it
			lines.push_back(line);
			clines.push_back(cline);
			line = "";
			cline.clear();
		} else {
			line += text[i];
			cline.push_back(colors[i]);
		}
	}
	if(line.size() != 0) {
		lines.push_back(line);
		clines.push_back(cline);
	}
}
void Buffer::change_text(std::string new_text) {
	this->text = new_text;
	this->update();
	this->window->update_lines();
	// TODO this call s ugly
	this->screen->draw_win(this->window);
	//this->window->redraw();
}/*}}}*/

/*{{{ Window*/
Window::Window(Screen *screen, Coords coords, unsigned int lines, unsigned int cols): __coords(coords), viewport(this) {
	this->screen = screen;
	this->buffer = new Buffer(this);
	__lines = lines;
	__cols = cols;
	this->viewport.first_line = 0;
	this->buf_cursor_coords = Coords();
};
void Window::update(Coords coords, unsigned int lines, unsigned int cols) {
	this->__coords = coords;
	this->__lines = lines;
	this->__cols = cols;
	this->update_lines();
	//?redraw window
}
void Window::update_lines() {
	// TODO: break_word not used! (always false)
	bool break_word = false;
	this->lines.clear();
	this->clines.clear();
	// TODO: USE POINTER INSTEAD
	std::vector<std::string>buf_lines = this->buffer->lines;
	std::vector<color::cvector_t>buf_clines = this->buffer->clines;
	std::string word = "";
	color::cvector_t cword;
	for(int li = 0; li < buf_lines.size(); ++ li) {
		for(int idx = 0; idx < buf_lines[li].size(); ++ idx) {
			char cur_ch = buf_lines[li][idx];
			color::ccell_t cur_ccell = buf_clines[li][idx];
			if(cur_ch != ' ') {
				word += cur_ch;
				cword.push_back(cur_ccell);
			} else {
				word += " ";
				cword.push_back(cur_ccell);
				// TODO: FIXME: if 1 word longer than line (also consider escape chars)
				// TODO: FIXME: if 1 word longer than line (also consider escape chars)
				// TODO: FIXME: if 1 word longer than line (also consider escape chars)
				if(lines.size() == 0) {
					lines.push_back(word);
					clines.push_back(cword);
				} else if(print_strlen(lines[lines.size() - 1]) + print_strlen(word) <= this->__cols) {
					lines[lines.size() - 1] += word;
					clines[clines.size() - 1].insert(
							clines[clines.size() - 1].end(),
							cword.begin(), cword.end()
							);
				} else {
					if(print_strlen(word) > this->__cols) {
						// TODO: implement word longer than line <<<
					} else {
						lines.push_back(word);
						clines.push_back(cword);
					}
				}
				word = "";
				cword.clear();
			}
		}
		if(word.size() != 0) {
			if(lines.size() == 0) {
				lines.push_back(word);
				clines.push_back(cword);
			} else if(print_strlen(lines[lines.size() - 1]) + print_strlen(word) <= this->__cols) {
				lines[lines.size() - 1] += word;
				// append cword to clines
				clines[clines.size() - 1].insert(
						clines[clines.size() - 1].end(),
						cword.begin(), cword.end()
						);
			} else {
				lines.push_back(word);
				clines.push_back(cword);
			}
			word = "";
			cword.clear();
		}
		//create new line
		lines.push_back("");
		clines.push_back(color::cvector_t());
	}
	if(lines.size()) {
		if(lines[lines.size() - 1].size() == 0) {
			lines.pop_back();
			clines.pop_back();
		}
	}
	this->__dplen.clear();
	__dplen.push_back(lines[0].size());
	for(int i = 1; i < lines.size(); ++ i) {
		__dplen.push_back(__dplen[i - 1] + lines[i].size());
	}
}
color::ctext_t ctext_parse(std::string ctext) {
	std::string active_color = color::RESET_COLOR;
	std::string text = "";
	color::cvector_t cvec;
	std::string _color = "";
	// multiple colors one after another
	bool color_secv = true;
	for(int i = 0; i < ctext.size(); ++i) {
		if(ctext[i] == '\033') {
			for(; ctext[i] != 'm' && i < ctext.size(); ++i) {
				_color += ctext[i];
			}
			if(ctext[i] != 'm') {
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
		text += ctext[i];
		cvec.push_back(color::ccell_t(active_color, color::RESET_COLOR));
	}
	return color::ctext_t(text, cvec);
}
void Window::buf_text(std::string new_text) {
	// by default scroll all the way to the top
	viewport.first_line = 0;
	buf_cursor_coords = Coords();
	this->buffer->change_text(new_text);
}
// change buffer's text with color new_text
void Window::ctext(std::string new_text) {
	viewport.first_line = 0;
	buf_cursor_coords = Coords();
	color::ctext_t ctext = ctext_parse(new_text);
	this->buffer->colors = ctext.second;
	this->buffer->change_text(ctext.first);
	// by default scroll all the way to the top
}
void Window::scroll(int num_lines) {
	int new_first_line = viewport.first_line + num_lines;
	if(new_first_line < 0) {
		new_first_line = 0;
		//UPDATE buf_cursor_coords;
	}
	if(new_first_line > this->lines.size() - 1) {
		new_first_line = this->lines.size() - 1;
	}
	if(viewport.first_line == new_first_line) {
		return;
	}
	viewport.first_line = new_first_line;
	screen->draw_win(this);
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
}/*}}}*/

/*{{{ Cursor*/
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
	this->move(win->__coords);
	// TODO: recover coords win_cursor_coords has to be implemented (in move_right() etc.)
	//this->move(win->__coords + win->win_cursor_coords);
}
// like <space> in vim
// TODO: WIP
void Cursor::____move_right() {
	Coords *bc = &window->buf_cursor_coords;
	bc->j++;
	if(bc->j > window->buffer->lines.size()) {
		window->buf_cursor_coords = Coords(bc->i + 1, 0);
	}
}
void Cursor::save_coords() {
	this->stored_coords[0] = this->coords();
}
void Cursor::restore_coords() {
	move(this->stored_coords[0]);
}/*}}}*/

namespace _Screen {/*{{{*/
	std::ofstream LOG("debug.log");
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
void Screen::printcl(Window *win, color::ctext_t cline) {
	for(int i = 0; i < cline.first.size(); ++i) {
		char chr = cline.first[i];
		color::ccell_t color = cline.second[i];
		std::cout << color.before << chr << color.after;
	}
	// TODO: string::size() is not accurate for escape chars, implement one
	const int spaces = (int)win->__cols - print_strlen(cline.first);
	for(int i = 0; i < spaces; ++ i) {
		std::cout << " ";
	}
}
void Screen::printcl(Window *win, std::string ctext) {
	printcl(win, ctext_parse(ctext));
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
				printcl(window, color::FG_MAGENTA + "@@@");
				++ c_coords.i;
				std::cout << color::RESET_COLOR;
				continue;
			}
		}
		if(line_idx == last_visible_line) {
			if(!last_line_visible && window->lines.size() >= 2) {
				printcl(window, color::hl_NonText + "@@@");
				++ c_coords.i;
				continue;
			}
		}
		if(line_idx < window->lines.size()) {
			printcl(window, make_pair(window->lines[line_idx], window->clines[line_idx]));
			//std::cout << printable_strlen(window->lines[line_idx]);
		} else {
			printcl(window, color::hl_EndOfBuffer + "~");
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
	int game_height = si * 0.6;
	int game_width = sj * 0.8;
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
	std::string debug_str = "RESIZE_DEBUG: "
		+ std::to_string(starti)
		+ ", "
		+ std::to_string(startj)
		+ " new win size";
	if(winn > 0) {
		this->windows[0]->ctext(
				debug_str
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
}/*}}}*/

// vim:fdm=marker:
