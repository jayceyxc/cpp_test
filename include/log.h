#ifndef LOG_H
#define LOG_H

#include <string>
#include <iostream>

using namespace std;

#define log_err(message) cerr << "file: " << __FILE__ << ", line: " << __LINE__ << ", function: " << __FUNCTION__ << ", compile time: " << __TIME__ << ", error message: " << message << endl

#define log_info(message) cout << "file: " << __FILE__ << ", line: " << __LINE__ << ", function: " << __FUNCTION__ << ", compile time: " << __TIME__ << ", error message: " << message << endl

/*
void log_err(string message) {
    cerr << "file: " << __FILE__ << ", line: " << __LINE__ << ", function: " << __FUNCTION__ << ", compile time: " << __TIME__ << ", error message: " << message << endl;
}

void log_err(const char* message) {
    cerr << "file: " << __FILE__ << ", line: " << __LINE__ << ", function: " << __FUNCTION__ << ", compile time: " << __TIME__ << ", error message: " << message << endl;
}
*/

#endif // LOG_H
