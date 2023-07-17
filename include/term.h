#include <termios.h>
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
