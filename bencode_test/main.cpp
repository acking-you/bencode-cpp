//
// Created by Alone on 2022-4-16.
//
#include "../../BenchMark/Timer.h"
#include <iostream>
#include <bencode.h>
#include <unordered_map>


using namespace bencode;
struct Student{
    std::string  name;
    int sid;
    std::unordered_map<std::string ,int> pp;
};

void to_bencode(Bencode& b,const Student& student){
    b["name"] = student.name;
    b["sid"] = student.sid;
    b["pp"] = student.pp;
}

void from_bencode(Bencode &b, Student &student){
    b["sid"].get_to(student.sid);
    b["name"].get_to(student.name);
    b["pp"].get_to(student.pp);
}

//void to_json(json & b,const Student& student){
//    b["name"] = student.name;
//    b["sid"] = student.sid;
//}
//
//void from_json(const json &b, Student &student){
//    b["sid"].get_to(student.sid);
//    b["name"].get_to(student.name);
//}

int main(){
    Timer t;
    Bencode b;
    b.append(323).append("fda").
    append(3232).
    append(Student{"fdaf",32,std::unordered_map<std::string ,int>{{"faf",32}}});
    std::cout<<b.to_string()<<"\n";
}