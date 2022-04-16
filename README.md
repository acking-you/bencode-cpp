# Bencode

A modern-cpp  bencode serialization/deserialization library.

* [Requirements](#requirements)
* [Installation](#installation)
* [Usage](#usage)
    * [Data types](#data-types)
    * [Serialization and Deserialization](#serialization-and-deserialization)
        * [Base Type](#base-type)
        * [Custom Type](#custom-type)
* [License](#license)
## Requirements

This library has no external dependencies and request cpp version higher than C++17.It's been tested on CLion with gcc10+.

**Note:** This library uses `std::variant` as the base type,so your cpp version needs to support it.

## Installation

I do not know how to create cmake libraries.😂

The entire project contains only `bencode.cpp` and `bencode.h`, so you can include these two files in your project and start using them.

Or you can use the static link library I provided at [lib folder](./lib).

and add the following two lines to the corresponding cmake file:

```cmake
link_directories({YOUR_link_DIR})
target_link_libraries({YOUR_Project_Name} bencode)
```

## Usage

### Data types

Bencode has four base data types: `integer`, `string`, `list`, and `dict`. These correspond to `int`, `std::string`, `std::vector<bencode::BObject>`, and `std::map<std::string, bencode::BObject>`, respectively. Since the data types are determined at runtime, these are all stored in a variant type called `BObject`.

More implementation details can be found in the BObject section of [bencode.h](./bencode.h)

### Serialization and Deserialization

#### Base Type

if it is `string`,`int`,`vector<baseType>`,`map<std::string,baseType>`.

You can use it directly like this:

```cpp
#include<bencode.h>

using namespace bencode;
int main(){
	Bencode b;
    //first method:
    b<<3243423;
    //second method:
    b["INT"] = 3243423;

    //Continuing to write directly will overwrite the previous content
    b<<"fsfjldjflsdfsd";

    b<<std::vector<int>{3,23,232};

    b<<std::map<std::string,int>{{"fds",32}};

    std::map<std::string,int> _map;
    //Deserialization
    b>>_map;
    
    //You can output directly to the ostream
    std::cout<<b;
    
}
```

I recommend using the second way for serialization, because `Bencode` only supports final serialization to a `dict`, so method one and method two are the same, except that method one hides the internal details.

If you want to get the bencode of basic types such as int directly, you can use `BEntity<int>` 。

```cpp
#include<bencode.h>
using namespace bencode;

int main(){
    BEntity<int> b;
    int p = 32323;
    b<<p;
    int q;
    b>>q;
    std::cout<<b<<std::endl;
}
```

**Note** : Method one serialization will empty the serialized content deposited in front of it, while method second, the way of tagging, does not empty the previous content.

#### Custom Type

For serialization and deserialization of custom types you need to overload the `to_bencode` and `from_bencode` functions.

```cpp
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
```

## License

This library is licensed under the [Apache License 2.0](./LICENSE)