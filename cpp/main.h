#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <iostream>
using std::cout;
using std::endl;
using std::cin;
using std::string;

void wait_at_end() {
	string s;
	std::getline(cin, s);
}

#endif // !MAIN_H
