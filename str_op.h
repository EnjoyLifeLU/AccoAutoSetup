#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <iostream>

std::vector<std::string> split_str(const std::string& str, const std::string& delim);
void replace_all(std::string& old_str, std::string& new_str, std::string const& old_value, std::string const& new_value);
void unique_vec_str(std::vector<std::string>& str);
std::string UnicodeToUTF8(const std::wstring& wstr);
std::wstring UTF8ToUnicode(const std::string& str);
char* CalcAlpha(char alpha, int num);