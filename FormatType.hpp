//FormatType.hpp
//head-source
#pragma once
#include <string>
#include <sstream>
#include <memory>
#include<iostream>
#include<chrono>
#include<codecvt>
#include<iomanip>

char *StrToChar(std::string sVar) {
	const char* c_sVar = sVar.c_str();
	char* cVar;
	cVar = _strdup(c_sVar);
	return cVar;
}

char _StrToChar(std::string sVar) {
	char cVar;
	std::stringstream str;
	str << sVar;
	str >> cVar;
	return cVar;
}

std::string CurrentTime(std::string str)
{
	std::stringstream ss;
	std::chrono::system_clock::time_point a = std::chrono::system_clock::now();      //ʱ����������
	time_t t1 = std::chrono::system_clock::to_time_t(a);				  //time_t���Ը�ʽ��
	ss << std::put_time(localtime(&t1), str.c_str());
	std::string str1 = ss.str();
	return str1;
}
