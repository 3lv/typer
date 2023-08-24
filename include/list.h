#ifndef LIST_H
#define LIST_H

#include <iostream>

/*{{{list.h */
#include <string>
#include <vector>

template <class T>
class List {
public:
	List *next;
	List *prev;
	T value;
	List();
	List(T val);
	List* insert(T val);
	List* append(T val);
	size_t size() const;
	template <class S>
	void operator=(std::vector<S> &v);
	void operator=(std::string str);
	List* operator+(ssize_t idx);
	List* operator-(ssize_t idx);
	T &operator[](ssize_t idx);
	// TODO: IMPLEMENT RECURSIVE DELETE
};
/*}}}*/

/*{{{ list.cpp implementation */
template <class T>
List<T>::List() {
	prev = 0;
	next = 0;
}
template <class T>
List<T>::List(T val) {
	value = val;
	prev = 0;
	next = 0;
}
template <class T> template <class S>
void List<T>::operator=(std::vector<S> &v) {
	if(v.size() == 0) {
		return;
	}
	List *it = this;
	for(size_t i = 0; i < v.size() - 1; ++i) {
		it->value = v[i];
		if(it->next == 0) {
			it->next = new List();
		}
		it->next->prev = it;
		it = it->next;
	}
	it->value = v[v.size() - 1];
	if(it->next) {
		delete(it->next);
	}
}
template<class T>
void List<T>::operator=(std::string str) {
	std::vector<char> vstr(str.begin(), str.end());
	operator=(vstr); // singurul rand pe care l am uitat
}
template <class T>
List<T>* List<T>::insert(T val) {
	List *new_node = new List(val);
	prev->next = new_node;
	new_node->prev = prev;
	prev = new_node;
	new_node->next = this;
	return new_node;
}
template <class T>
List<T>* List<T>::append(T val) {
	List *new_node = new List(val);
	next->prev = new_node;
	new_node->next = next;
	next = new_node;
	new_node->prev = this;
	return new_node;
}
template <class T>
size_t List<T>::size() const {
	size_t i = 0;
	const List *it = this;
	for(; it; it = it->next, i++);
	return i;
}
template <class T>
List<T>* List<T>::operator+(ssize_t idx) {
	List *it = this;
	if(idx >= 0) {
		for(size_t i = 0; i < (size_t)idx; ++i) {
			it = it->next;
		}
	} else if (idx < 0) {
		for(size_t i = 0; i < (size_t)abs((int)idx); ++i) {
			it = it->prev;
		}
	}
	return it;
}
template <class T>
List<T>* List<T>::operator-(ssize_t idx) {
	return this->operator+(-idx);
}
template <class T>
T &List<T>::operator[](ssize_t idx) {
	return (this->operator+(idx))->value;
}
/*}}}*/

#endif /* list.h */

// vi:fdm=marker
