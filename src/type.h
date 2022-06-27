//
// Created by Alone on 2022-5-7.
//

#ifndef TEST_BENCODE_TYPE_H
#define TEST_BENCODE_TYPE_H

#include "config.h"
#include <string>
#include <vector>

namespace bencode {
    enum class Error {
        ErrNum,
        ErrCol,
        ErrEpI,
        ErrEpE,
        ErrTyp,
        ErrIvd,
        NoError
    };
    enum class BType {
        BSTR,
        BINT,
        BLIST,
        BDICT
    };

    void perror(Error error_code, const char *info = nullptr);

    // type trait
    template<class T>
    struct isString {
        static const bool value = false;
    };
    template<>
    struct isString<std::string> {
        static const bool value = true;
    };
    template<class T>
    struct isInteger {
        static const bool value = false;
    };
    template<>
    struct isInteger<int> {
        static const bool value = true;
    };

    template<class T>
    struct isVector {
        static const bool value = false;
    };
    template<class T>
    struct isVector<std::vector<T>> {
        static const bool value = true;
    };
    template<class T>
    struct isMap {
        static const bool value = false;
    };
    template<class T>
    struct isMap<__DICT__<std::string, T>> {
        static const bool value = true;
    };
    template<class T>
    struct isCStr {
        static const bool value = false;
    };
    template<>
    struct isCStr<char *> {
        static const bool value = true;
    };
    template<class T>
    struct isBasicType {
        static const bool value = false;
    };
    template<>
    struct isBasicType<int> {
        static const bool value = true;
    };
    template<>
    struct isBasicType<std::string> {
        static const bool value = true;
    };
    template<>
    struct isBasicType<const char *> {
        static const bool value = true;
    };
}

#endif //TEST_BENCODE_TYPE_H
