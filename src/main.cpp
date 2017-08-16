#include <iostream>

#include "person.h"

using namespace std;

int main(int argc, char* argv[]) {
    Person person = new Person(18, "hello");
    cout << person.display();
}
