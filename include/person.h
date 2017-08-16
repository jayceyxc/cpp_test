#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

class Person {
    private:
        int age_;
        string name_;
    public:
        Person(int age, string name): age_(age), name_(name) {
            cout << "Person constructor" << endl;
        }

        ~Person() {

        }

        int getAge() {
            return this->age_
        }
        void setAge(int age) {
            this->age_ = age;
        }
        string getName() {
            return this->name_;
        }
        void setName(string name) {
            this->name_ = name;
        }
        string display();
};

#endif // PERSON_H
