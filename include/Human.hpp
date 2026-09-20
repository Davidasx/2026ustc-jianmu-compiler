#pragma once
#include <string>
class Human
{
private:
    /* data */
    std::string name_;
    int age_;
public:
    explicit Human(int age, const std::string name = "") : name_(name), age_(age) {};
    virtual ~Human();
    virtual std::string print() const;

};
