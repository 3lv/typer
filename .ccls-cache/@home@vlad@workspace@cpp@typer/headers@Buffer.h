#ifndef BUFFER_H
#define BUFFER_H
#include <vector>
#include <string>

class Buffer {
	private:
	public:
		unsigned int __lines;
		unsigned int __cols;
		Coords __coords;
		std::string text;
		std::vector<std::string>lines;
		Screen *screen;
		Buffer();
		void update();
};

#endif // BUFFER_H
