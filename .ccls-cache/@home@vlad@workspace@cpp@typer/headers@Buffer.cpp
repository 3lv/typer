#include "Buffer.h"

void Buffer::update() {
	screen->__update();
	this->__cols = screen->__cols;
	std::string word;
	lines.clear();
	for(int i = 0; i < text.size(); ++ i) {
		if(text[i] != ' ') {
			word += text[i];
		} else {
			if(lines[lines.size() - 1].size() + word.size() <= this->__cols) {
				lines[lines.size() - 1] += word;
			} else {
				lines.push_back(word);
			}
		}
	}
	if(lines[lines.size() - 1].size() + word.size() <= this->__cols) {
		lines[lines.size() - 1] += word;
	} else {
		lines.push_back(word);
	}
}
