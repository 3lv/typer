#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <chrono>
#include <thread>
#include <random>
#include "../include/screen.h"
using namespace std;
using namespace color;

const string TYPER_DIR = (string)getenv("HOME") + "/documents/typer/";
const string LANG_DIR = TYPER_DIR + "langs/";

class cp { // color pair
private:
public:
	ANSIcode fg;
	ANSIcode bg;
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
class User_colors {
private:
public:
	cp normal;
	cp current;
	struct {
		cp correct;
		cp incorrect;
	} typed;
} C;


vector<string> text_words;

string generate_text(string language, int lenght) {
	mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
	string lang_path = LANG_DIR + language + ".words";
	ifstream fin(lang_path);
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


string UP   = "\033[A";
string DOWN = "\033[B";
string RIGHT= "\033[C";
string LEFT = "\033[D";

bool running = true;
int incorrect_chars = 0;
int idx = 0;
string text = "";

void erase1(Screen *screen) { // TODO: outdated function, change it to match window size
	Coords c = screen->cursor->coords();
	if(c.j == 0 && c.i > 0) {
		screen->cursor->move(c.i - 1, screen->__cols);
		cout << C.normal.fg << text[idx];
	} else {
		cout << '\b' << C.normal.fg << text[idx] << '\b' << flush;
	}
	idx--;
}


int time_ms() {
	auto time = chrono::system_clock::now(); // get the current time
	auto since_epoch = time.time_since_epoch(); // get the duration since epoch
	auto millis = chrono::duration_cast<chrono::milliseconds>(since_epoch);
	int now = millis.count(); // just like java (new Date()).getTime();
	return now;
}

int main(int argc, char *argv[]) {
	if(argc == 1) {
		text = generate_text("english", 10);
	} else if(argc == 2) {
		text = generate_text("english", atoi(argv[1]));
	} else if(argc >= 3) {
		text = generate_text(argv[2], atoi(argv[1]));
	}
	/*
	 * cazan are mere
	*/
	int text_length = text.size();
	C.normal = cp(FG_WHITE);
	C.current = cp(RESET_COLOR);
	C.typed.correct = cp(FG_GREEN);
	C.typed.incorrect = cp(FG_RED);
	system("stty raw -echo");
	Screen screen;
	screen.draw();
	screen.create_win( Coords(1,1),
			2,
			screen.__cols - 1
			);
	screen.create_win( Coords(4,2),
			screen.__lines - 4,
			screen.__cols - 2
			);
	screen.windows[0]->buf_text(FG_BLACK_L + "Waiting..");
	screen.windows[1]->buf_text(C.normal.fg + text);
	screen.cursor->move(screen.windows[1]);
	text = " " + text + "    ";
	bool first_char = true;
	int starting_time = 0;
	while(running) {
		char k = getchar();
		if(k >= 32 && k <= 126) {
			idx++;
		} else {
		}
		if(first_char) {
			starting_time = time_ms();
			first_char = false;
			screen.windows[0]->buf_text(FG_BLUE + "Test started");
		}
		// Handle special characters
		if(k == 127 || k == 8) { // backspace
			if(idx > 0) {
				erase1(&screen);
			}
		} else if(k == 3) { // ^C
			screen.windows[0]->buf_text(FG_RED_L + "Test canceled:(");
			screen.cursor->move(screen.__lines, 0);
			system("stty cooked echo");
			return 1;
		} else if(k == 23) { // ^W
			if(idx > 0) {
				erase1(&screen);
				while(idx > 0 && text[idx] != ' ') {
					erase1(&screen);
				}
			}
		} else if(k >= 32 && k <= 126) { // Typeable character
			bool change_line = false;
			if(screen.cursor->coords().j == screen.__cols - 1) {
				change_line = true;
			}
			if(k == text[idx]) {
				cout << C.typed.correct.fg << text[idx] << flush;
			} else if(k != text[idx]) {
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
				cout << "\n\r" << flush;
			}
			cout << RESET_COLOR;
		}
	}
	// 1 char/ms = 12000 wpm
	float wpm = 12000.0 * text_length / (time_ms() - starting_time);
	float acc = 1.0 * text_length / (text_length + incorrect_chars) * 100;
	screen.windows[0]->buf_text( FG_GREEN_L + "Test finished!     "
			+ to_string(wpm) + FG_BLUE_L + " wpm     "
			+ FG_GREEN_L
			+ to_string(acc) + "%" + FG_BLUE_L + " acc"
			);
	// evident ca este o improvizatie
	//screen.cursor->move(screen.windows[1]->__coords.i + screen.windows[1]->buffer->lines.size(), 0);
	screen.cursor->move(screen.__lines, 0);
	char end_char = getchar();
	system("stty cooked; stty echo");
	return 0;
}
