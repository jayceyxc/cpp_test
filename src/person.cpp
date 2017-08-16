#include "person.h"

string Person::display() {
    stringstream ss;
    ss << "name: " << this->name_ << ", age: " << this->age_ << "\n";
    return ss.str();
}
