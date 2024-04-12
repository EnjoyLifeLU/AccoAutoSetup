//ReadConfig.h
//head
#pragma once
#include <string>

class Config
{
public:
	Config();
	~Config();
	void ReadConfigFile();

	std::string GetCondFilePath();

private:
	std::string condFilePath;
};

Config::Config()
{
}

Config::~Config()
{
}
