#ifndef BENCODE_BENCODE_H
#define BENCODE_BENCODE_H


#include <utility>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <map>

//#define U_DICT

#ifdef U_DICT
#define __DICT__  std::unordered_map
#else
#define __DICT__  std::map
#endif

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

    template<class T>
    class BEntity;

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

        // 隐式转化五件套
        BObject(std::string);

        BObject(const char *str);

        BObject(int v);

        BObject(LIST list);

        BObject(DICT dict);

        operator std::string();

        operator int();

        BObject &operator=(int);

        BObject &operator=(std::string);

        BObject &operator=(LIST);

        BObject &operator=(DICT);

        std::string *Str(Error *error_code = nullptr);

        int *Int(Error *error_code = nullptr);

        LIST *List(Error *error_code = nullptr);

        DICT *Dict(Error *error_code = nullptr);


        int Bencode(std::ostream &os);

        static std::shared_ptr<BObject> Parse(std::istream &in, Error *error);

        static int EncodeString(std::ostream &os, std::string_view val);

        static std::string DecodeString(std::istream &in, Error *error);

        static int EncodeInt(std::ostream &os, int val);

        static int DecodeInt(std::istream &in, Error *error);

    private:
        BType type;
        BValue value;

        static int getIntLen(int val);
    };

    // 用于直接序列化的工具模板类

    using LIST = BObject::LIST;
    using DICT = BObject::DICT;//方便外面少写一个区域名
    class Bencode;

    template<>
    class BEntity<LIST> {
        std::shared_ptr<BObject> object;
        BObject::LIST *list;
    public:
        BEntity() : object(std::make_shared<BObject>(LIST())) {
            list = object->List();
            if (list == nullptr)
                throw std::bad_alloc();
        }

        BEntity &add(BObject src) {
            list->push_back(std::make_shared<BObject>(std::move(src)));
            return *this;
        }

        int bencode(std::ostream &os) {
            return object->Bencode(os);
        }

        friend std::ostream &operator<<(std::ostream &os, const BEntity &entity) {
            entity.object->Bencode(os);
            return os;
        }

        friend std::istream &operator>>(std::istream &is, BEntity &entity) {
            Error error;
            entity.object = std::move(BObject::Parse(is, &error));
            if (error != Error::NoError) {
                perror(error);
                exit(-1);
            }
            return is;
        }
    };

    template<>
    class BEntity<DICT> {
    public:
        std::shared_ptr<BObject> object;
        BObject::DICT *dict;

        friend class Bencode;

        BEntity() : object(std::make_shared<BObject>(DICT())) {
            dict = object->Dict();
            if (dict == nullptr)
                throw std::bad_alloc();
        }

        BEntity &put(const std::string &key, BObject value) {
            if (!dict) {
                char msg[200];
                sprintf(msg, "dict nullptr,put dict error!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            dict->emplace(key, std::make_shared<BObject>(std::move(value)));
            return *this;
        }

        void clear() const {
            if (!dict) {
                char msg[200];
                sprintf(msg, "dict nullptr,clear dict error!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            dict->clear();
        }

        int bencode(std::ostream &os) {
            return object->Bencode(os);
        }

        friend std::ostream &operator<<(std::ostream &os, const BEntity &entity) {
            entity.object->Bencode(os);
            return os;
        }

        friend std::istream &operator>>(std::istream &is, BEntity &entity) {
            Error error;
            entity.object = std::move(BObject::Parse(is, &error));
            if (error != Error::NoError) {
                perror(error);
                exit(-1);
            }
            return is;
        }
    };

    template<>
    class BEntity<int> {
        std::shared_ptr<BObject> object;
        int *val;
    public:
        BEntity() : object(std::make_shared<BObject>(0)) {
            val = object->Int();
            if (val == nullptr)
                throw std::bad_alloc();
        }

        void set(int v) {
            *val = v;
        }

        int *data() {
            return val;
        }

        int bencode(std::ostream &os) {
            return object->Bencode(os);
        }

        friend std::ostream &operator<<(std::ostream &os, const BEntity &entity) {
            entity.object->Bencode(os);
            return os;
        }

        friend BEntity& operator>>(BEntity& b,int& val){
            if(b.val){
                val = *b.val;
            }else{
                perror(Error::ErrIvd,"val nullptr!");
            }
            return b;
        }

        friend BEntity& operator<<(BEntity& b,int val){
            if(b.val){
               *b.val = val;
            }else{
                perror(Error::ErrIvd,"val nullptr!");
            }
            return b;
        }

        friend std::istream &operator>>(std::istream &is, BEntity &entity) {
            Error error;
            entity.object = std::move(BObject::Parse(is, &error));
            if (error != Error::NoError) {
                perror(error);
                exit(-1);
            }
            return is;
        }
    };

    template<>
    class BEntity<std::string> {
        std::shared_ptr<BObject> object;
        std::string *val;
    public:
        BEntity() : object(std::make_shared<BObject>("")) {
            val = object->Str();
            if (val == nullptr)
                throw std::bad_alloc();
        }

        void set(std::string v) {
            *val = std::move(v);
        }

        const char *data() {
            return val->c_str();
        }

        int bencode(std::ostream &os) {
            return object->Bencode(os);
        }

        friend BEntity &operator<<(BEntity &b, std::string str) {
            if (b.val) {
                *b.val = std::move(str);
            } else {
                perror(Error::ErrIvd, "nullptr b.val");
            }
            return b;
        }

        friend BEntity &operator<<(BEntity &b, const char *str) {
            b << std::string(str);
            return b;
        }

        friend BEntity &operator>>(BEntity &b, std::string& str) {
            if (b.val) {
                str = std::move(*b.val);
            } else {
                perror(Error::ErrIvd, "nullptr b.val");
            }
            return b;
        }

        friend std::ostream &operator<<(std::ostream &os, const BEntity &entity) {
            entity.object->Bencode(os);
            return os;
        }

        friend std::istream &operator>>(std::istream &is, BEntity &entity) {
            Error error;
            entity.object = std::move(BObject::Parse(is, &error));
            if (error != Error::NoError) {
                perror(error);
                exit(-1);
            }
            return is;
        }
    };

    // type trait
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


// 函数声明
    template<class T>
    extern void to_bencode(Bencode &bEntity, const T &src);

    template<class T>
    extern void from_bencode(BEntity<T> &bEntity, T &src);

    // bencode解析类的本体
    class Bencode {
        BEntity<DICT> m_dict;
        std::string cur_key;
    public:
        Bencode() = default;

        Bencode &operator[](const std::string &key) {
            cur_key = key;
            return *this;
        }

        template<class T>
        Bencode &operator=(const T &src) {
            if (cur_key.empty()) {
                char msg[200];
                sprintf(msg, "operator= valid because of key empty!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            BObject obj;
            if constexpr(isBasicType<T>::value) {// 如果是基本类型直接走简单逻辑赋值
                obj = std::move(BObject(src));
            } else if constexpr(isVector<T>::value) {
                obj = std::move(LIST());
                putVector(obj, src);
            } else if constexpr(isMap<T>::value) {
                obj = std::move(DICT());
                putMap(obj, src);
            } else if constexpr(!isBasicType<T>::value) {// 如果是外界的复合类型，则走外界函数逻辑产生递归效果，并且注意需要生成一个新的dict来添加元素！
                std::string pre_key = cur_key;
                obj = std::move(DICT());
                auto new_dict = getNewDict(obj);

                auto old_dict = m_dict.dict;
                m_dict.dict = new_dict;
                to_bencode(*this, src);
                m_dict.dict = old_dict;
                cur_key = pre_key;
            }
            m_dict.put(cur_key, std::move(obj));
            return *this;
        }

        template<class T>
        void putMap(BObject &dest, const __DICT__<std::string, T> &src) {
            auto dict = dest.Dict();
            if (!dict) {
                char msg[200];
                sprintf(msg, "object change dict error in putMap\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            for (auto&&[k, v]: src) {
                BObject obj;
                if constexpr(isBasicType<T>::value) {
                    obj = BObject(v);
                } else if constexpr(isMap<T>::value) {
                    obj = std::move(DICT());
                    putMap(obj, v);
                } else if constexpr(isVector<T>::value) {
                    obj = std::move(LIST());
                    putVector(obj, v);
                } else if constexpr(!isBasicType<T>::value) {// 自定义类型情况，说明当前的哈希表value值是一个dict需要替换成这个dict然后再调用get函数即可
                    obj = std::move(DICT());
                    auto new_dict = getNewDict(obj);

                    auto pre_dict = m_dict.dict;
                    auto pre_key = cur_key;

                    m_dict.dict = new_dict;
                    to_bencode(*this, v);
                    m_dict.dict = pre_dict;
                    cur_key = pre_key;
                }
                dict->emplace(k, std::make_shared<BObject>(std::move(obj)));
            }
        }

        template<class T>
        void putVector(BObject &dest, const std::vector<T> &src) {
            auto list = dest.List();
            if (!list) {
                char msg[200];
                sprintf(msg, "object change list error in putVector\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            for (auto &&v: src) {
                BObject obj;
                if constexpr(isBasicType<T>::value) {
                    obj = BObject(v);
                } else if constexpr(isMap<T>::value) {
                    obj = std::move(DICT());
                    putMap(obj, v);
                } else if constexpr(isVector<T>::value) {
                    obj = std::move(LIST());
                    putVector(obj, v);
                } else if constexpr(!isBasicType<T>::value) {// 自定义类型情况，和getMap唯一的不同在于不需要维护之前的cur_key字段了
                    obj = std::move(DICT());
                    auto new_dict = getNewDict(obj);

                    auto pre_dict = m_dict.dict;

                    m_dict.dict = new_dict;
                    to_bencode(*this, v);
                    m_dict.dict = pre_dict;
                }
                list->template emplace_back(std::make_shared<BObject>(std::move(obj)));
            }
        }


        static DICT *getNewDict(BObject &src) {
            auto new_dict = src.Dict();
            if (!new_dict) {
                char msg[200];
                sprintf(msg, "dict change failed!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            return new_dict;
        }

        static LIST *getNewList(BObject &src) {
            auto new_list = src.List();
            if (!new_list) {
                char msg[200];
                sprintf(msg, "list change failed!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            return new_list;
        }

        template<class T>
        void getMap(__DICT__<std::string, T> &obj, BObject &src) {
            Error error;
            auto dict = src.Dict(&error);
            if (dict == nullptr) {
                char msg[200];
                sprintf(msg, "getMap failed!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(error, msg);
                exit(-1);
            }

            for (auto&&[k, v]: *dict) {
                if (!v) {
                    char msg[200];
                    sprintf(msg, "nullptr Exception!\r\n filename %s ,line %s", __FILE__, __LINE__);
                    perror(Error::ErrIvd, msg);
                    exit(-1);
                }
                T tmp;
                BObject &m_data = *v;
                if constexpr(isBasicType<T>::value) {
                    tmp = T(m_data);
                } else if constexpr(isMap<T>::value) {
                    getMap(tmp, m_data);
                } else if constexpr(isVector<T>::value) {
                    getVector(tmp, m_data);
                } else if constexpr(!isBasicType<T>::value) {// 自定义类型情况，说明当前的哈希表value值是一个dict需要替换成这个dict然后再调用get函数即可
                    auto new_dict = getNewDict(m_data);
                    auto pre = m_dict.dict;
                    m_dict.dict = new_dict;
                    tmp = get<T>();// recurse
                    m_dict.dict = pre;
                }
                obj.emplace(k, std::move(tmp));
            }
        }

        template<class T>
        void getVector(std::vector<T> &obj, BObject &src) {
            Error error;
            auto list = src.List(&error);
            if (list == nullptr) {
                char msg[200];
                sprintf(msg, "getVector failed!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(error, msg);
                exit(-1);
            }
            for (auto &&v: *list) {
                if (!v) {
                    char msg[200];
                    sprintf(msg, "nullptr Exception!\r\n filename %s ,line %s", __FILE__, __LINE__);
                    perror(Error::ErrIvd, msg);
                    exit(-1);
                }
                BObject &m_data = *v;
                T tmp;
                if constexpr(isBasicType<T>::value) {
                    tmp = T(m_data);
                } else if constexpr(isMap<T>::value) {
                    getMap(tmp, m_data);
                } else if constexpr(isVector<T>::value) {
                    getVector(tmp, m_data);
                } else {// 自定义类型情况
                    auto new_dict = getNewDict(m_data);
                    auto pre = m_dict.dict;
                    m_dict.dict = new_dict;
                    tmp = get<T>();// recurse
                    m_dict.dict = pre;
                }
                obj.emplace_back(std::move(tmp));
            }
        }

        //to call get<T>() implement this function
        template<class T>
        void get_to(T &dest) {
            dest = std::move(get<T>());
        }

        template<class T>
        T get() {
            if (!m_dict.dict) {
                char msg[200];
                sprintf(msg, "nullptr Exception!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            auto it = m_dict.dict->find(cur_key);
            T ret;
            if (it != m_dict.dict->end()) {
                BObject &object = *it->second;
                if constexpr(isMap<T>::value) {
                    getMap(ret, object);
                } else if constexpr(isVector<T>::value) {
                    getVector(ret, object);
                } else if constexpr(isBasicType<T>::value) {
                    ret = T(object);
                } else if constexpr(!isBasicType<T>::value) {// 如果是自定义类型，则说明此时object是一个dict，然后更改遍历的dict递归即可
                    auto new_dict = getNewDict(object);
                    auto pre = m_dict.dict;
                    m_dict.dict = new_dict;
                    from_bencode(*this, ret);
                    m_dict.dict = pre;
                }
            } else {
                perror(Error::ErrIvd, "at get<T>(): can't find key!");
                exit(-1);
            }
            return ret;
        }


        // overload operator<<
        template<class T>
        friend Bencode &operator<<(Bencode &bencode, const T &src) {
            bencode.m_dict.clear(); //把原先的数据先清空
            to_bencode(bencode, src);
            return bencode;
        }

        template<class T>
        friend Bencode &operator<<(Bencode &bencode, __DICT__<std::string, T> const &src) {
            bencode.m_dict.clear(); //把原先的数据先清空
            BObject &obj = *bencode.m_dict.object;
            bencode.template putMap(obj, src);
            return bencode;
        }


        template<class T>
        friend Bencode &operator<<(Bencode &bencode, std::vector<T> const &src) {
            bencode.m_dict.clear(); //把原先的数据先清空
            BObject list = LIST();
            bencode.template putVector(list, src);
            bencode.m_dict.put("LIST", list);
            return bencode;
        }

        friend Bencode &operator<<(Bencode &bencode, const std::string &src) {
            bencode.m_dict.clear();
            BObject str = src;
            bencode.m_dict.put("STR", str);
            return bencode;
        }

        friend Bencode &operator<<(Bencode &bencode, const int &src) {
            bencode.m_dict.clear();
            BObject integer = src;
            bencode.m_dict.put("INT", integer);
            return bencode;
        }

        friend Bencode &operator<<(Bencode &bencode, const char *src) {
            bencode.m_dict.clear();
            BObject str = src;
            bencode.m_dict.put("STR", str);
            return bencode;
        }

        // overload operator>>
        template<class T>
        friend Bencode &operator>>(Bencode &bencode, T &src) {
            from_bencode(bencode, src);
            return bencode;
        }

        template<class T>
        friend Bencode &operator>>(Bencode &bencode, __DICT__<std::string, T> &dest) {
            BObject &obj = *bencode.m_dict.object;
            bencode.template getMap(dest, obj);
            return bencode;
        }

        template<class T>
        friend Bencode &operator>>(Bencode &bencode, std::vector<T> &dest) {
            BObject &obj = *bencode.m_dict.object;
            bencode.template getVector(dest, obj);
            return bencode;
        }

        friend Bencode &operator>>(Bencode &bencode, std::string &dest) {
            BObject &obj = *bencode.m_dict.object;
            dest = std::move(std::string(obj));
            return bencode;
        }

        friend Bencode &operator>>(Bencode &bencode, int &dest) {
            BObject &obj = *bencode.m_dict.object;
            dest = obj;
            return bencode;
        }


        // overload stream operator<< and operator>>
        friend std::ostream &operator<<(std::ostream &os, Bencode &bencode) {
            bencode.m_dict.bencode(os);
            return os;
        }

        friend std::istream &operator>>(std::istream &is, Bencode &bencode) {
            Error error;
            bencode.m_dict.object = std::move(BObject::Parse(is, &error));

            if (error != Error::NoError) {
                perror(error);
                exit(-1);
            }
            bencode.m_dict.dict = bencode.m_dict.object->Dict(&error);
            if (error != Error::NoError) {
                char msg[200];
                sprintf(msg, "in operator>>,stream not a dict!\r\n filename %s ,line %s", __FILE__, __LINE__);
                perror(Error::ErrIvd, msg);
                exit(-1);
            }
            return is;
        }
    };
}

#endif //BENCODE_BENCODE_H
