//
// Created by Alone on 2022-5-7.
//

#include "BObject.h"
#include "BEntity.hpp"
#include <sstream>
#include <iostream>

using std::string;
using bencode::BObject;

bencode::BObject::operator std::string() {
    Error error;
    auto str = Str(&error);
    if (!str) {
        perror(error,"can't convert to string!");
        exit(-1);
    }
    return *str;
}

bencode::BObject::operator int() {
    Error error;
    auto val = Int(&error);
    if (!val) {
        perror(error,"can't convert to integer!");
        exit(-1);
    }
    return *val;
}


std::string *bencode::BObject::Str(Error *error_code) {
    if (this->type_ != BType::BSTR) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<string>(&this->value_);
}

int *bencode::BObject::Int(Error *error_code) {
    if (this->type_ != BType::BINT) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<int>(&this->value_);
}

BObject::LIST *bencode::BObject::List(Error *error_code) {
    if (this->type_ != BType::BLIST) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<LIST>(&this->value_);
}

BObject::DICT *bencode::BObject::Dict(Error *error_code) {
    if (this->type_ != BType::BDICT) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<DICT>(&this->value_);
}

//recursive descent bencode
int bencode::BObject::Bencode(std::ostream &os) {
    int wLen = 0;
    if (!os) {
        return wLen;
    }
    void *val;
    switch (this->type_) {
        case BType::BSTR:
            val = this->Str();
            if (val)
                wLen += EncodeString(os, *(string *) val);
            break;
        case BType::BINT:
            val = this->Int();
            if (val) {
                wLen += EncodeInt(os, *(int *) val);
            }
            break;
        case BType::BLIST:
            val = this->List();
            os << 'l';
            if (val) {
                LIST &t = *(LIST *) val;
                for (auto &&item: t) {
                    if (item)
                        wLen += item->Bencode(os);
                    else {
                        perror(Error::ErrIvd, "pointer null! line 127");
                    }
                }
            }
            os << 'e';
            wLen += 2;
            break;
        case BType::BDICT:
            val = this->Dict();
            os << 'd';
            if (val) {
                DICT &t = *(DICT *) val;
                for (auto &&[k, v]: t) {
                    wLen += EncodeString(os, k);
                    wLen += v->Bencode(os);
                }
            }
            os << 'e';
            wLen += 2;
            break;
    }

    return wLen;
}

//recursive descent parsing
std::shared_ptr<BObject> bencode::BObject::Parse(std::istream &in, Error *error) {
    auto x = in.peek();
    BObject *obj;
    if (std::isdigit(x)) {//parse string
        auto str = DecodeString(in, error);
        if ((error && *error != Error::NoError) || str.empty()) {
            return nullptr;
        }
        obj = new BObject();
        obj->type_ = BType::BSTR;
        obj->value_ = str;
    } else if (x == 'i') {//parse int
        auto val = DecodeInt(in, error);
        if (error && *error != Error::NoError) {
            return nullptr;
        }
        obj = new BObject();
        obj->type_ = BType::BINT;
        obj->value_ = val;
    } else if (x == 'l') {//parse GetList
        in.get();
        LIST list;
        do {
            if (in.peek() == 'e') {
                in.get();
                break;
            }
            auto ele = Parse(in, error);
            if ((error && *error != Error::NoError) || !ele) {
                return nullptr;
            }
            list.emplace_back(std::move(ele));
        } while (true);
        obj = new BObject();
        obj->type_ = BType::BLIST;
        obj->value_ = std::move(list);
    } else if (x == 'd') {//parse GetDict
        in.get();
        DICT dict;
        do {
            if (in.peek() == 'e') {
                in.get();
                break;
            }
            auto key = DecodeString(in, error);
            if ((error && *error != Error::NoError) || key.empty()) {
                return nullptr;
            }
            auto val = Parse(in, error);
            if ((error && *error != Error::NoError) || !val) {
                return nullptr;
            }
            dict.emplace(std::move(key), std::move(val));
        } while (true);
        obj = new BObject();
        obj->type_ = BType::BDICT;
        obj->value_ = std::move(dict);
    } else {
        if (error)*error = Error::ErrIvd;
        return nullptr;
    }
    if (error)*error = Error::NoError;
    return std::shared_ptr<BObject>(obj);
}

//converse string to stringstream and to Parse
bencode::Bencode bencode::BObject::parse(std::string text) {
    std::istringstream reader(std::move(text));
    Error error;
    auto ptr = Parse(reader,&error);
    if(error!=Error::NoError){
        throw std::runtime_error("parse error");
    }
    return bencode::Bencode(ptr);
}



int bencode::BObject::getIntLen(int val) {
    int ret = 0;
    if (val < 0) {
        ret = 1;
        val *= -1;
    }
    do {
        ret++;
        val /= 10;
    } while (val);
    return ret;
}

int bencode::BObject::EncodeString(std::ostream &os, std::string_view val) {
    int len = val.size();
    if (len <= 0)
        return 0;
    os << len;
    int wLen = getIntLen(len);
    os << ":";
    wLen++;
    os << val;
    wLen += len;

    return wLen;
}


std::string bencode::BObject::DecodeString(std::istream &in, Error *error) {
    int len;
    in >> len;
    if (len == 0) {
        if (error)*error = Error::ErrNum;
        return "";
    }

    char b;
    in.get(b);
    if (b != ':') {
        if (error)*error = Error::ErrCol;
        return "";
    }

    string ret_v;
    ret_v.reserve(len);  //TODO 提前分配内存防止中途分配内存
    int rdlen = 0;
    char x;
    while (rdlen < len && in.get(x)) {
        ret_v.push_back(x);
        rdlen++;
    }

    if (rdlen != len) {
        if (error)*error = Error::ErrIvd;
        return "";
    }
    if (error)*error = Error::NoError;
    return ret_v;
}

int bencode::BObject::EncodeInt(std::ostream &os, int val) {
    int wLen = 0;
    os << 'i';
    wLen++;
    os << val;
    wLen += getIntLen(val);
    os << 'e';
    wLen++;

    return wLen;
}

int bencode::BObject::DecodeInt(std::istream &in, Error *error) {
    char x;
    in.get(x);
    if (x != 'i') {
        if (error)*error = Error::ErrEpI;
        return 0;
    }

    int val;
    in >> val;
    in.get(x);

    if (x != 'e') {
        if (error)*error = Error::ErrEpI;
        return val;
    }
    if (error)*error = Error::NoError;
    return val;
}

BObject &bencode::BObject::operator=(int v) {
    value_ = v;
    type_ = BType::BINT;
    return *this;
}

BObject &bencode::BObject::operator=(string str) {
    value_ = std::move(str);
    type_ = BType::BSTR;
    return *this;
}

BObject &bencode::BObject::operator=(BObject::LIST list) {
    type_ = BType::BLIST;
    value_ = std::move(list);
    return *this;
}

BObject &bencode::BObject::operator=(DICT dict) {
    type_ = BType::BDICT;
    value_ = std::move(dict);
    return *this;
}

bencode::BObject::BObject(std::string v) : value_(std::move(v)), type_(BType::BSTR) {

}

bencode::BObject::BObject(int v) : value_(v), type_(BType::BINT) {
}

bencode::BObject::BObject(BObject::LIST list) : value_(std::move(list)), type_(BType::BLIST) {

}

bencode::BObject::BObject(BObject::DICT dict) : value_(std::move(dict)), type_(BType::BDICT) {

}

bencode::BObject::BObject(const char *str) : BObject(string(str)) {

}

#define PRINT_NEXT_LINE(var)  obj.append(string(var,' '));

void bencode::BObject::get_json(int curRowLen, std::string &obj) {
    switch (type_) {
        case BType::BSTR:{
            auto str = Str();
            if(!str){
                NULL_ERROR(to_string,STR)
            }
            auto src_str = R"(")"+*str+R"(")";
            obj.append(src_str);
            break;
        }
        case BType::BINT:{
            auto integer = Int();
            if(!integer){
                NULL_ERROR(to_string,INT)
            }
            auto src_str = std::to_string(*integer);
            obj.append(src_str);
            break;
        }
        case BType::BLIST:{
            auto list = List();
            if(!list){
                NULL_ERROR(to_string,LIST)
            }
            obj.append("[");
            curRowLen += 1;
            for(int i=0;i<list->size();i++){
                auto& item = list->at(i);
                if(item->type_==BType::BDICT){
                    obj.push_back('\n');
                    PRINT_NEXT_LINE(curRowLen)
                    item->get_json(curRowLen, obj);
                }else{
                    item->get_json(curRowLen, obj);
                }
                if(i!=list->size()-1){
                    obj.append(", ");
                    curRowLen += 2;
                }
            }
            obj.append("]");
            curRowLen+=1;
            break;
        }
        case BType::BDICT:{
            auto dict = Dict();
            if(!dict){
                NULL_ERROR(to_string,DICT)
            }
            obj.append("{\n");
            PRINT_NEXT_LINE(curRowLen)
            for(auto&&[k,v]:*dict){
                auto format_keyStr = R"(")"+k+R"(":)";
                obj.append(format_keyStr);
                auto newLen = curRowLen+format_keyStr.size();
                if(v->type_==BType::BDICT){
                    obj.push_back('\n');
                    PRINT_NEXT_LINE(newLen)
                    v->get_json(newLen, obj);
                }else{
                    v->get_json(curRowLen, obj);
                }
                obj.append(",\n");
                PRINT_NEXT_LINE(curRowLen)
            }
            obj.append("}");
            break;
        }
    }

}

std::string bencode::BObject::to_string() {
    string obj;
    get_json(0, obj);
    return obj;
}