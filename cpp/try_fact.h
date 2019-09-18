#pragma once

#include "main.h"

size_t fact(size_t n) {
	auto factn = [](auto k, auto n) -> size_t {
		return n <= 1 ? 1 : n * k(k, n - 1);
	};
	return factn(factn, n);
}

auto fix = [](auto f) {
	auto k = [f](auto x) {
		return f([x](auto y) -> int { return x(x)(y); });
	};
	return k(k);
};

auto factn = [](auto f) {
	return [f](int n) {
		return n <= 1 ? 1 : (n * f(n - 1));
	};
};

auto factp = fix(factn);

void run() {
	cout << factp(10) << endl;
	
	wait_at_end();
}
