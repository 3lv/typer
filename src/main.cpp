#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <chrono>
#include <thread>
#include <random>
#include <signal.h>
#include "../include/screen.h"

#ifdef _WIN32
# include <conio.h>
#else
# define getch() getchar()
#endif

using namespace std;
using namespace color;

#ifdef _WIN32
const string HOME = ".";
const string TYPER_DIR = HOME + "/.";
const string LANG_DIR = "langs/";
#else
const string HOME = (string)getenv("HOME");
const string TYPER_DIR = HOME + "/documents/typer/";
const string LANG_DIR = TYPER_DIR + "langs/";
#endif

const string DEBUG_DIR = HOME + "/workspace/cpp/typer/build/debug.log";
//std::ofstream LOG(DEBUG_DIR);

struct cp { // color pair
	std::string fg;
	std::string bg;
	cp() {
		this->fg = RESET_COLOR;
		this->bg = "";
	}
	cp(ANSIcode fg) {
		this->fg = fg;
		// TODO change BG_BLACK to screen->colors.background
		this->bg = BG_BLACK;
	}
	cp(ANSIcode fg, ANSIcode bg) {
		this->fg = fg;
		this->bg = bg;
	}
};
struct User_colors {
	cp normal = cp(FG_WHITE);
	cp current = cp(RESET_COLOR);
	struct {
		cp correct = cp(FG_GREEN);
		cp incorrect = cp(FG_RED);
	} typed;
} C;

vector<string> text_words;


string UP   = "\033[A";
string DOWN = "\033[B";
string RIGHT= "\033[C";
string LEFT = "\033[D";

bool running = true;
int incorrect_chars = 0;
Coords c;
int idx = 0;
string text = "";
string language = "english";
size_t word_count = 10;
size_t text_length = 0;

void erase1(Screen *screen) { // TODO: outdated function, change it to match window size
	if(idx == 0) {
		return;
	}
	if(c.j == 0) {
		c.i--;
		c.j = screen->windows[1]->lines[c.i].size() - 1;
		screen->cursor->move(c.i + screen->windows[1]->__coords.i, c.j + screen->windows[1]->__coords.j);
		cout << C.normal.fg << text[idx] << '\b' << flush;
		idx--;
	} else {
		cout << '\b' << C.normal.fg << text[idx] << '\b' << flush;
		c.j--;
		idx--;
	}
}

void parse_args(int argc, char *argv[]) {
	for(int i = 1; i < argc; ++ i) {
		int num = atoi(argv[i]);
		if(num) {
			word_count = num;
		} else {
			language = argv[i];
		}
	}
}

string generate_text(string language, int lenght) {
	mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
	string lang_file = LANG_DIR + language + ".words";
	ifstream fin(lang_file);
	if(!fin.good()) {
		return "";
	}
	vector <string> words;
	string word;
	while(fin >> word) {
		words.push_back(word);
	}
	for(int i = 0; i < lenght; ++ i) {
		text_words.push_back(words[rng() % words.size()]);
	}
	string text = "";
	for(int i = 0; i < lenght - 1; ++ i) {
		text += text_words[i];
		text += " ";
	}
	text += text_words[lenght - 1];
	return text;
}

int time_ms() {
	auto time = chrono::system_clock::now(); // get the current time
	auto since_epoch = time.time_since_epoch(); // get the duration since epoch
	auto millis = chrono::duration_cast<chrono::milliseconds>(since_epoch);
	int now = millis.count(); // just like java (new Date()).getTime();
	return now;
}

int main(int argc, char *argv[]) {
	parse_args(argc, argv);
	text = generate_text(language, word_count);
	text_length = text.size();
	if(text_length == 0) {
		return 1;
	}
	Screen *screen = &Screen::screen;
	Cursor *cursor = screen->cursor;
	screen->init();
	int game_height = (screen->__lines + 1) * 0.6;
	int game_width = (screen->__cols + 1) * 0.8;
	game_width = min(game_width, (int)text_length);
	game_width = min(game_width, 100);
	int starti = (screen->__lines - game_height) / 2;
	int startj = (screen->__cols - game_width) / 2;
	Window *win_header = screen->create_win( Coords(starti, startj),
			2,
			game_width
			);
	Window *win_main = screen->create_win( Coords(starti + 2, startj),
			game_height - 2,
			game_width
			);
	win_header->buf_text(LIGHT + FG_BLACK + "Type anything to start the test..");
	win_main->buf_text(text);
	cursor->move(win_main);
	text = " " + text + "    ";
	bool first_char = true;
	int starting_time = 0;
	while(running) {
		char k = getch();
		if(first_char) {
			starting_time = time_ms();
			first_char = false;
			win_header->buf_text(LIGHT + FG_BLUE + "Test started");
		}
		// Handle special characters
		if(k == 127 || k == 8) { // backspace
			if(idx > 0) {
				erase1(screen);
			}
		} else if(k == 3) { // ^C
			win_header->buf_text(LIGHT + FG_RED + "Test canceled:(");
			return 1;
		} else if(k == 23) { // ^W
			if(idx > 0) {
				erase1(screen);
				while(idx > 0 && text[idx] != ' ') {
					erase1(screen);
				}
			}
		} else if(k >= 32 && k <= 126) { // Typeable character
			bool change_line = false;
			if(c.j == win_main->lines[c.i].size() - 1) {
				change_line = true;
			}
			if(k == text[idx + 1]) {
				idx++;
				c.j++;
				cout << C.typed.correct.fg << text[idx] << flush;
			} else if(k != text[idx + 1]) {
				idx++;
				c.j++;
				incorrect_chars ++;
				if(text[idx] == ' ') {
					cout << BG_RED << text[idx] << flush;
				} else {
					cout << C.typed.incorrect.fg << text[idx] << flush;
				}
			}
			if(idx == text_length) {
				running = false;
			}
			if(change_line == true) {
				++ c.i;
				c.j = 0;
				// move considering win_main.viewport
				cursor->move(c.i + win_main->__coords.i - win_main->viewport.first_line, c.j + win_main->__coords.j);
			}
			cout << RESET_COLOR;
		}
	}
	// 1 char/ms = 12000 wpm
	float wpm = 12000.0 * text_length / (time_ms() - starting_time);
	float acc = 1.0 * text_length / (text_length + incorrect_chars) * 100;
	win_header->buf_text( FG_GREEN + "Test finished!     "
			+ LIGHT + to_string(wpm) + LIGHT + FG_BLUE + " wpm     "
			+ LIGHT + FG_GREEN + to_string(acc) + "%" + LIGHT + FG_BLUE + " acc"
			);
	//screen->cursor->move(screen->__lines, 0);
	char end_char = getch();
	return 0;
}
