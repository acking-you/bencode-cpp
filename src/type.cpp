//
// Created by Alone on 2022-5-7.
//

#include "type.h"
#include <iostream>
using std::cerr;

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