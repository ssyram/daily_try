#pragma once
#ifndef TEST_MINUS_ARRAY_H
#define TEST_MINUS_ARRAY_H

#include <iostream>
using std::cout;
using std::endl;

const char * const basic_arr[] = {
	"abc",
	"123",
	"jkl",
	"ijk",
	"123",
	"123",
	"jkl",
	"ijk",
	"123",
	"123",
	"jkl",
	"ijk",
	"123",
	"123",
	"jkl",
	"ijk",
	"123",
	"123",
	"jkl",
	"ijk",
	"123",
	"123",
	"jkl",
	"ijk",
	"123",
};

#include <functional>
using std::function;

template <typename FuncTp>
struct SimpleFunctionWrapper {
	FuncTp f;
	SimpleFunctionWrapper(FuncTp f) : f(f) { }

	template <typename ...Args>
	decltype(f(std::declval<Args>()...)) operator()(Args &&...args) {
		return f(std::forward<Args>(args) ...);
	}
};

const char * const * arr = basic_arr + 25;

void run() {
	cout << arr[-25] << endl;

	int k = 10;
	++k;
}

#endif // !TEST_MINUS_ARRAY_H
