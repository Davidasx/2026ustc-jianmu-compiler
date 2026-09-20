#include "Student.hpp"
#include "Human.hpp"
#include "cstring"
#include <sstream>
#include <string>

Student::~Student() {
    printf("Student destructor called\n");
}

std::string Student::print() const {
    std::string res;
    std::stringstream ss;
    ss << Human::print();
    ss << "I'm from " << school_ << "\n";
    ss >> res;
    return res;
}