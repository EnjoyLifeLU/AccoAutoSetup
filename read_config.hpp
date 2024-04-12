#pragma once
#include "easylogging++.h"
#include "str_op.h"
#include "Config.h"
#include <vector>
#include <windows.h> 
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")

std::string condFilePath;
std::string testerType;
std::string testerID;
std::string hanlderID;
std::string programPath;
std::string datalogPath;
std::string datalogPath2;
std::string batFilePath;
std::string special;

std::vector<std::string> specialVec;

//int testerID_flag;
int bools;
int SPLEnable;
int SampleCount;
double Yield;
int QAType;
int SetQANumber;
int MaxQANumber;
int QAType_EQCRT;
int SetQANumber_EQCRT;
int MaxQANumber_EQCRT;

//
std::string  test_ip;

std::string CheckIP();
std::string CheckHostName();

int init_config() {
	//const char ConfigFile[] = "C:cpp/Example_test6/Debug/config.ini";
	const char ConfigFile[] = "config.ini";
	Config configSettings(ConfigFile);

	std::string defaultValue = "";
	std::string condFilePathValue = "C:/AccoAutoSetup/AutoSetupSH/cond.asc";
	std::string programPathValue = "D:/TEMP_PGM";
	std::string datalogPathValue = "D:/test/DataLog";
	std::string batFilePathValue = "C:/AccoAutoSetup/AutoSetupSH/AutoSetupSH.bat";
	int boolsValue = 1;
	int SPLEnableValue = 1;
	int SampleCountValue = 300;
	double YieldValue = 85;

	test_ip = configSettings.Read("test_ip", defaultValue);

	condFilePath = configSettings.Read("cond_file_path", condFilePathValue);
	programPath = configSettings.Read("program_path", programPathValue);

	datalogPath = configSettings.Read("datalog_path", datalogPathValue);
	datalogPath2 = configSettings.Read("datalog_path2", datalogPathValue);
	special = configSettings.Read("special", defaultValue);
	specialVec = split_str(special, ",");
	batFilePath = configSettings.Read("bat_file_path", batFilePathValue);
	
	bools = configSettings.Read("bools", boolsValue);
	SPLEnable = configSettings.Read("spl_enable", SPLEnableValue);
	SampleCount = configSettings.Read("sample_count", SampleCountValue);
	Yield = configSettings.Read("yield", YieldValue);

	QAType = configSettings.Read("qa_type", 2);
	SetQANumber = configSettings.Read("set_qa_number", 50);
	MaxQANumber = configSettings.Read("max_qa_number", 200);
	QAType_EQCRT = configSettings.Read("qa_type_bin8_rt", 2);
	SetQANumber_EQCRT = configSettings.Read("set_qa_number_bin8_rt", 1);
	MaxQANumber_EQCRT = configSettings.Read("max_qa_number_bin8_rt", 200);

	testerID = CheckHostName();
	if (configSettings.Read("tester_id", defaultValue) != "") {
		testerID = configSettings.Read("tester_id", defaultValue);
	}
	if (testerID != "get hostname error") {
		hanlderID = configSettings.Read(testerID, defaultValue);
		//testerID_flag = configSettings.Read(testerID + "_flag", 0);
		testerType = testerID.substr(0, 7);
	} else {
		return -1;
	}
	return 0;
}

std::string CheckIP()
{
	WSADATA wsaData;
	char name[255];
	char* ip;
	PHOSTENT hostinfo;
	std::string ipStr;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)
	{
		if (gethostname(name, sizeof(name)) == 0)
		{
			if ((hostinfo = gethostbyname(name)) != NULL)
			{
				ip = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
				ipStr = ip;
			}
		}
		WSACleanup();
	}
	return ipStr;
}

std::string CheckHostName()
{
	WSADATA wsaData;
	char name[255];

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)
	{
		if (gethostname(name, sizeof(name)) == 0)
		{
			return name;
		}
		WSACleanup();
	}
	return "get hostname error";
}

