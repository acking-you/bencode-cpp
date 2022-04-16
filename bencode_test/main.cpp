//
// Created by Alone on 2022-4-16.
//

#include <iostream>
#include <bencode.h>

using namespace bencode;
struct Student{
    std::string  name;
    int sid;
};
void to_bencode(Bencode& b,const Student& student){
    b["name"] = student.name;
    b["sid"] = student.sid;
}

void from_bencode(Bencode &b, Student &student){
    b["sid"].get_to(student.sid);
    b["name"].get_to(student.name);
}

int main(){
    Bencode b;
    Student student;
    student.name = "刘xx";
    student.sid = 3232323;
    b<<student;
    b>>student;
    std::cout<<student.name<<std::endl;
    std::cout<<student.sid<<std::endl;
}