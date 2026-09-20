#include "Human.hpp"
#include <string>
#include <sstream>

Human::~Human() {
    printf("Human destructor called\n");
}

std::string Human::print() const {
    std::string res;
    std::stringstream ss;
    ss << "My name is " << name_ << " and I am " << age_ << " years old\n";
    ss >> res;
    return res;
}