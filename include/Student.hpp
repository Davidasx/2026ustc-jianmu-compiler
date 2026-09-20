#pragma once

#include "Human.hpp"
#include <string>

class Student : public Human
{
private:
    /* data */
    std::string school_;
public:
    Student() = delete; 
    explicit Student(int age, const std::string name = "", const std::string school = "") : Human(age, name), school_(school) {};
    virtual ~Student() override;
    virtual std::string print() const override;
};