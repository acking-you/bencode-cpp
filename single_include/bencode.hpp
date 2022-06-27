//
// Created by Alone on 2022-6-17.
//

#ifndef TEST_BENCODE_BENCODE_H
#define TEST_BENCODE_BENCODE_H
#include <string>
#include <vector>
#include<memory>
#include<iostream>
#include<variant>

#ifdef U_DICT
#define __DICT__  std::unordered_map
#include <unordered_map>
#else
#define __DICT__  std::map
#include <map>
#endif

namespace bencode{
    using std::cerr;
    using std::cout;
    using std::string;
    using std::vector;

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

    inline void perror(Error error_code, const char *info = nullptr){
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

    //pre statement
    template<class T>
    class BEntity;

    class Bencode;

    class BObject {
    public:
        using LIST = std::vector<std::shared_ptr<BObject>>;
        using DICT = __DICT__<std::string, std::shared_ptr<BObject>>;
        using BValue = std::variant<int, std::string, LIST, DICT>;

        friend class BEntity<LIST>;

        friend class BEntity<int>;

        friend class BEntity<std::string>;

        friend class BEntity<DICT>;

        BObject() = default;

        // 构造函数转化五件套
        explicit BObject(std::string){

        }

        explicit BObject(const char *str){

        }

        explicit BObject(int v){

        }

        explicit BObject(LIST list){

        }

        explicit BObject(DICT dict){

        }

        //强转重载
        operator std::string(){

        }

        operator int(){

        }

        BObject &operator=(int){

        }

        BObject &operator=(std::string){

        }

        BObject &operator=(LIST){

        }

        BObject &operator=(DICT){

        }

        std::string *Str(Error *error_code = nullptr){

        }

        int *Int(Error *error_code = nullptr){

        }

        LIST *List(Error *error_code = nullptr){

        }

        DICT *Dict(Error *error_code = nullptr){

        }


        int Bencode(std::ostream &os){

        }

        static std::shared_ptr<BObject> Parse(std::istream &in, Error *error){

        }


        static int EncodeString(std::ostream &os, std::string_view val){

        }

        static std::string DecodeString(std::istream &in, Error *error){

        }

        static int EncodeInt(std::ostream &os, int val){

        }

        static int DecodeInt(std::istream &in, Error *error){

        }

        template<class T>
        T value() {
            T *ptr;
            if constexpr(isInteger<T>::value) {
                ptr = Int();
                if (!ptr) {
                    throw std::runtime_error("BObject value() error,change to int failed!");
                }
            } else if constexpr(isString<T>::value) {
                ptr = Str();
                if (!ptr) {
                    throw std::runtime_error("BObject value() error,change to string failed!");
                }
            } else if constexpr(isVector<T>::value) {
                ptr = List();
                if (!ptr) {
                    throw std::runtime_error("BObject value() error,change to List failed!");
                }
            } else if constexpr(isMap<T>::value) {
                ptr = Dict();
                if (!ptr) {
                    throw std::runtime_error("BObject value() error,change to Dict failed!");
                }
            } else {
                //TODO 需要改成支持任意自定义类型的获取
                throw std::runtime_error("BObject value() error,no exist type");
            }
            return *ptr;
        }
        void get_json(int curRowLen, std::string & obj){

        }
        std::string to_string(){

        }
    private:
        static int getIntLen(int val){

        }
    private:
        int space_num_{};
        BType type_;
        BValue value_;
    };
}



#endif //TEST_BENCODE_BENCODE_H
