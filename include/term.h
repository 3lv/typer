#include <termios.h>

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

class Term {
private:
	termios __previous_mode;
public:
	enum class Mode {
		REVERT, CE, CNE, NCE, NCNE
	};
	void change_mode(Mode mode) {
		//tcgetattr( STDIN_FILENO, &original);
		//STDIN_FILENO = 0
		termios changed;
		if(mode == Mode::REVERT) { // revert term mode
			changed = __previous_mode;
		} else if(mode == Mode::CE) {
			tcgetattr( 0, &__previous_mode);
			changed = __previous_mode;
			changed.c_lflag |= ICANON;
			changed.c_lflag |= ECHO;
		} else if(mode == Mode::NCE) { // convert to noncanonical and echo
			tcgetattr( 0, &__previous_mode);
			changed = __previous_mode;
			changed.c_lflag &= ~ICANON;
			changed.c_lflag |= ECHO;
		} else if(mode == Mode::NCNE) { // convert to noncanonical and nonecho
			tcgetattr( 0, &__previous_mode);
			changed = __previous_mode;
			changed.c_lflag &= ~(ICANON | ECHO);
		}
		// these should already be set as 1 and 0
		/*
		   changed.c_cc[VMIN] = 1;
		   changed.c_cc[VTIME] = 0;
		   */
		tcsetattr( 0, TCSANOW, &changed);
	}
};
