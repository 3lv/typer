#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <random>
using namespace std;

const string TYPER_DIR = (string)getenv("HOME") + "/documents/typer/";
const string LANG_DIR = TYPER_DIR + "langs/";

/*
enum class KEYS {
	backspace = 8,
	backspacealt = 127,
	linefeed = 10, // ^J
	carriagereturn = 13, // ^M
	ctrlc = 3,
	ctrlw = 23,
};
*/

map<string, string> CCODES = {
	{"black", "30"},
	{"red", "31"},
	{"green", "32"},
	{"yellow", "33"},
	{"blue", "34"},
	{"magenta", "35"},
	{"cyan", "36"},
	{"white", "37"},
	{"lightblack", "30;1"},
	{"lightred", "31;1"},
	{"lightgreen", "32;1"},
	{"lightyellow", "33;1"},
	{"lightblue", "34;1"},
	{"lightmagenta", "35;1"},
	{"lightcyan", "36;1"},
	{"lightwhite", "37;1"},
	{"reset", "0"},
};

string escape_color(string color) {
	string p = "\033[";
	string s = "m";
	return p + CCODES[color] + s;
}

// \033[31;1;4m

struct color_map {
	string current = "cyan";
	string normal = "clear";
	struct typed {
		string incorrect = "red";
		string correct = "green";
	}typed;
}COLORS_MAP;

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

void goto_col(int col) {
	cout << '\r';
	for(int i = 0 ; i < col; ++ i) {
		cout << RIGHT;
	}
	cout << flush;
}

bool running = true;
int incorrect_chars = 0;
int idx = 0;
string text = "";
string spaces = "                              ";

void erase1() {
	cout << '\b' << text[idx] << '\b' << flush;
	idx--;
}

void update_header(string text) {
	cout << "\033[F";
	cout << escape_color("reset");
	cout << text;
	cout << spaces << "\n";
	cout << escape_color("reset");
	goto_col(idx);
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
	system("clear");
	int text_length = text.size();
	cout << escape_color("lightblack") + "Type the first char to start the timer" << "\n";
	cout << escape_color("reset");
	cout << "\r";
	cout << text << "\r";
	text = " " + text + "    ";
	system("stty raw");
	bool first_char = true;
	int starting_time = 0;
	while(running) {
		char k = getchar();
		if(k >= 32 && k <= 126) {
			idx++;
		} else if(k == 30 ){
			idx += 3;
			erase1();
			erase1();
			erase1();
		} else {
			idx += 2;
			erase1(); // cancel the 2 characters forming the special character (^C/^H)
			erase1();
		}
		if(first_char) {
			starting_time = time_ms();
			first_char = false;
			update_header(escape_color("blue") + "Test started");
		}
		// Handle special characters
		if(k == 127 || k == 8) { // backspace
			if(idx > 0) {
				erase1();
			}
			continue;
		}
		if(k == 3) { // ^C
			system("stty cooked");
			system("clear");
			return 1;
		}
		if(k == 23) { // ^W
			if(idx > 0) {
				erase1();
				while(idx > 0 && text[idx] != ' ') {
					erase1();
				}
			}
		}
		// Typeable character
		if(k >= 32 && k <= 126) {
			if(k == text[idx]) {
				cout << "\b" << escape_color(COLORS_MAP.typed.correct) << text[idx] << flush;
			} else if(k != text[idx]) {
				incorrect_chars ++;
				if(text[idx] == ' ') {
					cout << "\b" << "\033[41m" << text[idx] << flush;
				}
				cout << "\b" << escape_color(COLORS_MAP.typed.incorrect) << text[idx] << flush;
			}
			if(idx == text_length) {
				running = false;
			}
		}
		cout << escape_color("reset");
	}
	// 1 char/ms = 12000 wpm
	float wpm = 12000.0 * text_length / (time_ms() - starting_time);
	float acc = 1.0 * text_length / (text_length + incorrect_chars) * 100;
	update_header(escape_color("lightgreen") + "Test finished!     "
			+ to_string(wpm) + escape_color("lightblue") + " wpm     "
			+ escape_color("lightgreen")
			+ to_string(acc) + "%" + escape_color("lightblue") + " acc"
			);
	cout << escape_color("reset");
	cout << "\n\r";
	char end_char = getchar();
	cout << "\n\r";
	system("stty cooked");
	return 0;
}
