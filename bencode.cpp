// Created by Alone on 2022-4-3.
//

#include "bencode.h"

#include <iostream>

using std::cerr;
using bencode::Error;
using bencode::BType;
using bencode::BObject;
using std::string;

void bencode::perror(bencode::Error error_code, const char *info) {
    if (info)cerr << info << "  ";
    switch (error_code) {
        case Error::ErrNum:
            cerr << "expect num\n";
            break;
        case Error::ErrCol:
            cerr << "expect colon\n";
            break;
        case Error::ErrEpI:
            cerr << "expect char i\n";
            break;
        case Error::ErrEpE:
            cerr << "expect char e\n";
            break;
        case Error::ErrTyp:
            cerr << "wrong type\n";
            break;
        case Error::ErrIvd:
            cerr << "invalid bencode\n";
            break;
        default:
            cerr << "no error\n";
    }
}

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
    if (this->type != BType::BSTR) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<string>(&this->value);
}

int *bencode::BObject::Int(Error *error_code) {
    if (this->type != BType::BINT) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<int>(&this->value);
}

BObject::LIST *bencode::BObject::List(Error *error_code) {
    if (this->type != BType::BLIST) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<LIST>(&this->value);
}

BObject::DICT *bencode::BObject::Dict(Error *error_code) {
    if (this->type != BType::BDICT) {
        if (error_code)*error_code = Error::ErrTyp;
        return nullptr;
    }
    if (error_code)*error_code = Error::NoError;
    return get_if<DICT>(&this->value);
}

//recursive descent bencode
int bencode::BObject::Bencode(std::ostream &os) {
    int wLen = 0;
    if (!os) {
        return wLen;
    }
    void *val;
    switch (this->type) {
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
        obj->type = BType::BSTR;
        obj->value = str;
    } else if (x == 'i') {//parse int
        auto val = DecodeInt(in, error);
        if (error && *error != Error::NoError) {
            return nullptr;
        }
        obj = new BObject();
        obj->type = BType::BINT;
        obj->value = val;
    } else if (x == 'l') {//parse list
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
        obj->type = BType::BLIST;
        obj->value = std::move(list);
    } else if (x == 'd') {//parse dict
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
        obj->type = BType::BDICT;
        obj->value = std::move(dict);
    } else {
        if (error)*error = Error::ErrIvd;
        return nullptr;
    }
    if (error)*error = Error::NoError;
    return std::shared_ptr<BObject>(obj);
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
    value = v;
    type = BType::BINT;
    return *this;
}

BObject &bencode::BObject::operator=(string str) {
    value = std::move(str);
    type = BType::BSTR;
    return *this;
}

BObject &bencode::BObject::operator=(BObject::LIST list) {
    type = BType::BLIST;
    value = std::move(list);
    return *this;
}

BObject &bencode::BObject::operator=(DICT dict) {
    type = BType::BDICT;
    value = std::move(dict);
    return *this;
}

bencode::BObject::BObject(std::string v) : value(std::move(v)), type(BType::BSTR) {

}

bencode::BObject::BObject(int v) : value(v), type(BType::BINT) {
}

bencode::BObject::BObject(BObject::LIST list) : value(std::move(list)), type(BType::BLIST) {

}

bencode::BObject::BObject(BObject::DICT dict) : value(std::move(dict)), type(BType::BDICT) {

}

bencode::BObject::BObject(const char *str) : BObject(string(str)) {

}








