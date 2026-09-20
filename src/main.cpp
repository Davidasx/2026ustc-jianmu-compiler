#include <iostream>
#include <list>
#include <string>
#include <stdio.h>
#include "Human.hpp"
#include "Student.hpp"
#include <vector>

class check{
    public:
    explicit check(Student* student){
        printf("Student object created\n");
    }
};

int main(int argc, char** argv){
    if(argc == 1 || std::string(argv[1]) != "-t"){
        std::cerr << "error: invalid argument\n";
        exit(-1);
    }
    printf("Hello, from stl_debug!\n");
    std::vector<Human*> vec;
    Human human(25, "John Doe");
    Student student(20, "Jane Doe", "MIT");
    vec.push_back(&human);
    vec.push_back(&student);
    for (const auto& h : vec) {
        std::cout << h->print();
    }
    // auto student1 = new Student();   //TODO: Error 1 : why?
    auto student1 = static_cast<Student*>(vec.back());
    // check check1 = check(vec.back());   //TODO: Error 2 : why?
    check check1 = check(student1);

    // std::list<int> list;
    // list.push_back(1);
    // for(auto i: list){
    //     list.remove(i);              //TODO: Error 3 : why?
    // }
    return 0;
}
