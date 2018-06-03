
#ifndef _KERNAL_UTIL_H_
#define _KERNAL_UTIL_H_

#include <string>
#include <vector>

std::vector<std::string> strSplit(const std::string s, const std::string& delim);

int getLocalIP(char* outip);     // 获取IP地址
int getLocalMac(char* mac);      // 获取mac地址

unsigned int getCurrentTime();
#endif
