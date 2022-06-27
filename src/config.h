//
// Created by Alone on 2022-5-7.
//

#ifndef TEST_BENCODE_CONFIG_H
#define TEST_BENCODE_CONFIG_H

#ifdef U_DICT
#define __DICT__  std::unordered_map
#include <unordered_map>
#else
#define __DICT__  std::map
#include <map>
#endif

#endif //TEST_BENCODE_CONFIG_H
