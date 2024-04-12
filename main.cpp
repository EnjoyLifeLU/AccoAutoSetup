//log head
//#define ELPP_UNICODE
//#define ELPP_THREAD_SAFE
//#define  WIN32_LEAN_AND_MEAN
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
//cust head
#include "CondInfo.hpp"
#include "GlobalVariables.h"
#include "FormatType.hpp"
#include "Command.h"
#include "Units.h"
#include "read_config.hpp"
#include "gpibapi.h"
//lib head
#include <fmt\format.h>
#include <boost\filesystem\path.hpp>
#include <boost\filesystem.hpp>
#include <iostream>
#include <string>
#include <curl\curl.h>
//#include <cstring>
#include <Windows.h>
#include <stdio.h>
#include <windows.h>
#include <memory>
#include <io.h>
#include <conio.h>
#include <imm.h>
#include <algorithm>
#include <regex>
//#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib") 
#pragma comment (lib, "imm32.lib")
#pragma comment (lib, "libcurl.lib")

//#define  WIN32_LEAN_AND_MEAN
#define DEF_PORT 1234
#define BUF_SIZE 64
#define IsFileExist(lpszFileName) (::GetFileAttributes((lpszFileName))!=0xFFFFFFFF)

SdrRecord cSdrRecord;
wrrRecord cWrrRecord;
MirRecord cMirRecord;
RunMode RunModes;
HANDLE g_hEvent;

bool initializeWinsock() {
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void cleanupWinsock() {
	WSACleanup();
}

struct ResponseData {
	std::string response;
	std::string httpStatus;
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, ResponseData* responseData) {
	size_t totalSize = size * nmemb;

	// 处理HTTP响应头
	std::string responseStr(static_cast<char*>(contents), totalSize);
	if (responseStr.length() >= 4 && responseStr.substr(0, 4) == "HTTP") {
		size_t statusPos = responseStr.find(" ");
		if (statusPos != std::string::npos) {
			size_t statusEndPos = responseStr.find(" ", statusPos + 1);
			if (statusEndPos != std::string::npos) {
				responseData->httpStatus = responseStr.substr(statusPos + 1, statusEndPos - statusPos - 1);
			}
		}
	}

	// 处理响应数据
	responseData->response.append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

ResponseData LoadSummaryReportforAC(const std::string& summaryFile, const std::string& lotNO, const std::string& step) {
	std::string url = "http://192.168.220.59:40000/cimes/MPG.asmx/LoadSummaryReportforAC";
	std::string parameters = "summaryFile=" + summaryFile + "&lotNO=" + lotNO + "&step=" + step;
	ResponseData responseData;

	CURL* curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}

		curl_easy_cleanup(curl);
	}

	return responseData;
}

std::string tcpClient(const std::string& ipAddress, int port, std::string& message) {
	// 初始化 Winsock
	if (!initializeWinsock()) {
		std::cerr << "Failed to initialize winsock" << std::endl;
		return "";
	}

	// 创建 Socket
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket" << std::endl;
		cleanupWinsock();
		return "";
	}

	// 连接到服务器
	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		std::cerr << "Failed to connect to server" << std::endl;
		closesocket(clientSocket);
		cleanupWinsock();
		return "";
	}

	std::string response;

	do {
		// 发送消息给服务器
		if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
			std::cerr << "Failed to send message" << std::endl;
			closesocket(clientSocket);
			cleanupWinsock();
			return "";
		}

		// 接收服务器的响应
		char buffer[4096];
		memset(buffer, 0, sizeof(buffer));

		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived < 0) {
			std::cerr << "Failed to receive response" << std::endl;
			closesocket(clientSocket);
			cleanupWinsock();
			return "";
		}

		// 打印服务器的响应
		std::string serverResponse(buffer, bytesReceived);
		std::cout << "Server response: " << serverResponse << std::endl;
		response += serverResponse;

	} while (response != "true" && response != "false" && response != "err");

	// 关闭Socket
	closesocket(clientSocket);
	cleanupWinsock();

	return response;
}

void strtrim(char *str)
{
	char *str_c = str;
	int i, j = 0;
	for (i = 0; str[i] != '\0'; i++)
	{
		if (str[i] != ' ')
			str_c[j++] = str[i];
	}
	str_c[j] = '\0';
	str = str_c;
}

void main(int argc,char* argv[])
{
	el::Configurations conf("my_log.conf");
	el::Loggers::reconfigureAllLoggers(conf);

	std::string cmd;
	std::string DatalogFileName;
	std::string strPGS;
	std::string inputFile;
	std::string outputFile;
	char pcPath[MAX_PATH] = { 0 };
	char pcAPPPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, pcAPPPath, MAX_PATH);
	char sDrive[_MAX_DRIVE] = { 0 };
	char sDir[_MAX_DIR] = { 0 };
	char sFname[_MAX_FNAME] = { 0 };
	char sExt[_MAX_EXT] = { 0 };
	_splitpath_s(pcAPPPath, sDrive, sDir, sFname, sExt);
	sprintf_s(pcPath, "%s%sAcco_Soft_Settings.dll", sDrive, sDir);
	char pcAccoDll[MAX_PATH] = { 0 };

	g_hEvent = CreateEventW(NULL, 0, 0, LPCWSTR("Hi"));
	SetEvent(g_hEvent);
	if (g_hEvent)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError()){
			MessageBox(GetForegroundWindow(), "AutoLoad程序还运行中，不可重复启动", "Run Error", 0);
			return;
		}
	}
	std::string strProcessName = "control.exe";
	DWORD nPid = NULL;
	FindProcess(strProcessName, nPid);
	if (nPid == NULL) {
		MessageBox(GetForegroundWindow(), "程序启动失败，请检查STS8200控制平台是否打开", "Run Error", 0);
		return;
	}
	//system((char*)fmt::format("{}{}/ft_server.bat", sDrive, sDir).c_str());

	if (init_config() == -1) {
		MessageBox(GetForegroundWindow(), "程序启动失败，配置文件异常，请检查配置文件中的 #所有机台信息 ", "Run Error", 0);
		return;
	}

	/*if (IsFileExist(pcPath))
	{
		return;
	}*/

	//normal test or retest
	unsigned short curQAEnable;
	unsigned short curQAType;
	int curSetQANumber;
	int curMaxQANumber;
	int curLotNumber;
	double curDefaultPassRate;
	std::string command_rd;
	std::string NorR;
	std::string RTInfo;
	std::string strCin = "";
	bool QA = true;
	while (1) {
		system("cls");
		std::cout << "Current Handler: " << hanlderID << "\n";
		printf("Please select test mode: \n");
		printf("0:DPAT------(DPAT Test) \n");
		printf("1:FT--------(Normal Test) \n");
		printf("2:RT--------(ReTest) \n");
		printf("3:EQCRT-----(EQC ReTest) \n");
		CapsLock();
		//std::cin >> strCin;
		getline(std::cin, strCin);
		trim(strCin);
		if (strCin.length() > 1) {
			continue;
		}
		if (strCin == "0") {
			NorR = "DPAT";
			QA = false;
		}
		else if (strCin == "1") {
			NorR = "FT";
		}
		else if (strCin == "2") {
			NorR = "RT";
			QA = false;
		}
		else if (strCin == "3") {
			NorR = "EQCRT";
		}
		if (strCin == "0") {
			curQAEnable = 0;
			curQAType = 2;
			curSetQANumber = 0;
			curMaxQANumber = 0;
			curLotNumber = NULL;
			curDefaultPassRate = NULL;
		}
		if (strCin == "1") {
			curQAEnable = 1;
			curQAType = QAType;
			curSetQANumber = SetQANumber;
			curMaxQANumber = MaxQANumber;
			curLotNumber = NULL;
			curDefaultPassRate = NULL;
		}
		if (strCin == "2") {
			curQAEnable = 1;
			curQAType = QAType;
			curSetQANumber = SetQANumber;
			curMaxQANumber = MaxQANumber;
			curLotNumber = NULL;
			curDefaultPassRate = NULL;
		}
		else if (strCin == "3") {
			curQAEnable = 1;
			curQAType = QAType_EQCRT;
			curSetQANumber = SetQANumber_EQCRT;
			curMaxQANumber = MaxQANumber_EQCRT;
			curLotNumber = NULL;
			curDefaultPassRate = NULL;
		}
		if (strCin == "0" || strCin == "1" || strCin == "2" || strCin == "3") {
			break;
		}
	}

	//call system cmd
	std::string LotID;
	printf("Please enter the LotID: \n");
	while (1) {
		CapsLock();
		//std::cin >> LotID;
		getline(std::cin, LotID);
		trim(LotID);
		if (LotID != "")
		{
			printf("Successful entered the LotID\n");
			break;
		}
		system("cls");
		std::cout << "Current Handler: " << hanlderID << "\n";
		printf("Please select test mode: \n");
		printf("0:DPAT------(DPAT Test) \n");
		printf("1:FT--------(Normal Test) \n");
		printf("2:RT--------(ReTest) \n");
		printf("3:EQCRT-----(EQC ReTest) \n");
		fmt::print("{}\n", strCin);
		printf("enter the LotID Error\n");
		printf("Please re-enter the LotID: \n");
	}
	//call bat
	if (bools) {
		command_rd = batFilePath + " " + hanlderID + " " + LotID;
		system(command_rd.c_str());
	}
	//--------------------------------------------------------------------------------
	if (IsFileExist((LPCSTR)condFilePath.c_str())) {
		if (LoadCond2(condFilePath)) {
			if ("Y" == configCondFile.SYSmsg) {
				cMirRecord.LOT_ID = StrToChar(configCondFile.owner);
				cMirRecord.LOT_ID = StrToChar(configCondFile.lotid);
				cMirRecord.SBLOT_ID = StrToChar(configCondFile.clotid);
				configCondFile.prodid;
				cMirRecord.FLOW_ID = StrToChar(configCondFile.step);
				//configCondFile.tform;
				cMirRecord.MODE_COD = RunModes;
				//configCondFile.BINDefine;
				//configCondFile.RETESTBIN;
				cUMSAutoLoad.PSGNAME = StrToChar(configCondFile.Program);
				//configCondFile.Programver;
				cUMSAutoLoad.PSGPATH = StrToChar(configCondFile.pgmpath);
				//configCondFile.tpirno;
				//configCondFile.oper;
				cMirRecord.TST_TEMP = StrToChar(configCondFile.temperature);
				cMirRecord.OPER_NAM = StrToChar(configCondFile.OPID);
				configCondFile.externalno;
				printf("Load Cond.asc Successful!\n ");
			}
			else {
				LOG(INFO) << "Run Error: Load Cond.asc failed!";
				MessageBox(GetForegroundWindow(), "Load Cond.asc failed!", "Run Error", 0);
				return;
			}

			/*int port = 10000;
			std::string message = ":SO1000:LOADCFG " + configCondFile.externalno + ".cfg";

			std::string response = tcpClient(test_ip, port, message);
			if (response != "true") {
				LOG(INFO) << "Run Error: Load handler cfg failed!";
				MessageBox(GetForegroundWindow(), "Load handler cfg failed!", "Run Error", 0);
				return;
			}*/

			/*CopyFile((LPCSTR)condFilePath.c_str(), "X:\\CIM\\InterfaceSO1000\\Macro\\cond.asc", false);*/
		}
	}
	else {
		LOG(INFO) << "Run Error: Load Cond.asc no exist!";
		MessageBox(GetForegroundWindow(), "Run Error: Load Cond.asc no exist!", "Run Error", 0);
		return;
	}

	if (NorR == "RT" || NorR == "EQCRT") {
		std::vector<std::string> files;
		std::vector<std::string> files2;
		const std::string path = fmt::format("X:\\{}\\Datalog", configCondFile.owner);
		std::regex reg0(fmt::format("{}-{}-{}-FT-*.*", configCondFile.Program, configCondFile.lotid, configCondFile.step));
		std::regex reg1(fmt::format("{}-{}-{}-RT[0-9][A-Z]-*.*", configCondFile.Program, configCondFile.lotid, configCondFile.step));
		std::regex reg2("RT(\\w)(\\w)");

		std::string search_path = path + "\\*";
		WIN32_FIND_DATAA file_data;
		HANDLE dir_handle = FindFirstFile(search_path.c_str(), &file_data);
		if (dir_handle == INVALID_HANDLE_VALUE) {
			std::cerr << "Invalid directory path!" << std::endl;
			return;
		}

		do {
			if (strcmp(file_data.cFileName, ".") != 0 && strcmp(file_data.cFileName, "..") != 0) {
				std::string filename = file_data.cFileName;
				if (std::regex_match(filename, std::regex(reg0))) {
					files2.push_back(filename);
				}
			}
		} while (FindNextFileA(dir_handle, &file_data));

		if (files2.size() == 0)
		{
			MessageBox(GetForegroundWindow(), "未找到FT测试数据，请确认是否进行FT测试再进行RT测试", "Run Error", 0);
			return;
		}

		do {
			if (strcmp(file_data.cFileName, ".") != 0 && strcmp(file_data.cFileName, "..") != 0) {
				std::string filename = file_data.cFileName;
				if (std::regex_match(filename, std::regex(reg1))) {
					files.push_back(filename);
				}
			}
		} while (FindNextFileA(dir_handle, &file_data));
		
		
		std::string lastFileName = "";
		std::string lastDigit = "1";
		std::string lastAlpha = "A";
		for (const std::string& file : files) {
			std::smatch result;
			std::cout << file << std::endl;
			std::regex_search(file, result, reg2);
			std::string digit = result[1];
			std::string alpha = result[2];

			if (digit.compare(lastDigit) == 1) {
				lastDigit = digit;
				lastAlpha = alpha;
				lastFileName = file;
			} 
			else if (digit.compare(lastDigit) == 0) {
				lastDigit = digit;
				if (alpha.compare(lastAlpha) == 0 || digit.compare(lastDigit) == 1) {
					lastAlpha = alpha;
					lastFileName = file;
				}
			}
		}

		if (lastFileName != "") {
			std::string choice;
			std::cout << "\n\n\n";
			std::cout << "请确认当前重测是否为接测(Y/N):";
			std::cin >> choice;

			if (choice == "Y" || choice == "y") {
				int c = (int(lastAlpha[0]) + 1);
				char re[2];
				re[0] = char(c);
				re[1] = '\0';
				RTInfo = NorR + lastDigit + re;

			}
			else if (choice == "N" || choice == "n") {
				RTInfo = NorR + std::to_string(atoi(lastDigit.c_str()) + 1) + lastAlpha;
			}
		}
		else {
			RTInfo = NorR + "1A";
		}
	}
	else {
		RTInfo = NorR;
	}

	//// TODO:check handler configration
	/*char* Buffer = NULL;
	do {
		Buffer = NULL;
		Buffer = GPIBQuery(":HANDLER:ID?");
		strtrim(Buffer);
	} while (Buffer == NULL);

	if (strcmp(Buffer, configCondFile.externalno.c_str())) {
		MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("handler configration error!  GPIB Query :{}:{}:", Buffer, configCondFile.externalno).c_str(), "Run Error", 0);
		return;
	}

	do {
		Buffer = NULL;
		Buffer = GPIBQuery(":TEMP:SETPOINT?");
		strtrim(Buffer);
	} while (Buffer == NULL);

	if (strcmp(Buffer, configCondFile.temperature.c_str())) {
		MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("handler configration error!  GPIB Query :{}:{}:", Buffer, configCondFile.externalno).c_str(), "Run Error", 0);
		return;
	}*/

	// chech if it is special package
	if (std::find(specialVec.begin(), specialVec.end(), configCondFile.externalno) != specialVec.end()) {
		if (configCondFile.step == "FT1" || configCondFile.step == "FT2" || configCondFile.step == "FT3") QA = false;
		datalogPath = datalogPath2;
	}

	//sprintf_s(pcAccoDll, "%s%sSts8200Interface.dll", sDrive, sDir);
	if (IsFileExist(pcPath))
	{
		HINSTANCE hDll = LoadLibrary(pcPath);
		if (hDll!= NULL)
		{
			typedef int(* DLLFUN)(char*,int);
			DLLFUN dllFun = (DLLFUN)GetProcAddress(hDll, "GetAutoTestModule");
			if (dllFun!= NULL)
			{
				dllFun(pcAccoDll, MAX_PATH);
			}
			FreeLibrary(hDll);
		}
	}
	if (strlen(pcAccoDll) < strlen("Sts8200Interface.dll"))
	{
		LOG(INFO) << "未找到Sts8200Interface.dll!";
		MessageBox(GetForegroundWindow(), "未找到Sts8200Interface.dll!", "Run Error", 0);
		return;
	}
	if (!IsFileExist(pcAccoDll))
	{
		MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("[{}] is not exist.", pcAccoDll).c_str(), "Run Error", 0);
		return;
	}
	HINSTANCE hDll = LoadLibrary(pcAccoDll);
	if (hDll != NULL)
	{
		//Set Control oper
		typedef int(_stdcall * SWITCHUSER)(char *, char *);
		SWITCHUSER SwitchUser = (SWITCHUSER)GetProcAddress(hDll, "SwitchUser");
		if (NULL == SwitchUser)
		{
			LOG(INFO) << "Function [SwitchUser] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SwitchUser] is not exist.", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		//Set lotinfo\datalogname\outfile
		typedef int(_stdcall * CREATENEWLOTFILE)();
		CREATENEWLOTFILE CreateNewLotFile = (CREATENEWLOTFILE)GetProcAddress(hDll, "CreateNewLotFile_Acco");
		if (NULL == CreateNewLotFile)
		{
			LOG(INFO) << "Function [CreateNewLotFile_Acco] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [CreateNewLotFile_Acco] is not exist.", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * APPENDSTDFLOTMSG)(char*, int, char*, byte, byte);
		APPENDSTDFLOTMSG AppendStdfLotMsg = (APPENDSTDFLOTMSG)GetProcAddress(hDll, "AppendStdfLotMsg3");
		if (NULL == AppendStdfLotMsg)
		{
			LOG(INFO) << "Function [AppendStdfLotMsg] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [AppendStdfLotMsg] is not exist.", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * SETDATALOGFILENAME)(char*);
		SETDATALOGFILENAME SetDatalogFileName = (SETDATALOGFILENAME)GetProcAddress(hDll, "SetDatalogFileName");
		if (NULL == SetDatalogFileName)
		{
			LOG(INFO) << "Function [SetDatalogFileName] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetDatalogFileName] is not exist.", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * SETCURRENTSTATION)(int);
		SETCURRENTSTATION SetCurrentStation = (SETCURRENTSTATION)GetProcAddress(hDll, "SetCurrentStation");
		if (SetCurrentStation == NULL)
		{
			LOG(INFO) << "Function [SetCurrentStation] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetCurrentStation] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * RUNTESTUI)();
		RUNTESTUI RunTestui = (RUNTESTUI)GetProcAddress(hDll, "RunTestui");
		if (RunTestui == NULL)
		{
			LOG(INFO) << "Function [RunTestui] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [RunTestui] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * ENABLEQATEST)();
		ENABLEQATEST EnableQATest = (ENABLEQATEST)GetProcAddress(hDll, "EnableQATest");
		if (NULL == EnableQATest)
		{
			LOG(INFO) << "Function [EnableQATest] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [EnableQATest] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * SETPGSFILENAME)(char *);
		SETPGSFILENAME SetPgsFileName = (SETPGSFILENAME)GetProcAddress(hDll, "SetPgsFileName");
		if (NULL == SetPgsFileName)
		{
			LOG(INFO) << "Function [SetPgsFileName] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetPgsFileName] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * SETRECENTYIELD)(int, int, double);
		SETRECENTYIELD SetRecentYield = (SETRECENTYIELD)GetProcAddress(hDll, "SetRecentYield");
		if (NULL == SetRecentYield)
		{
			LOG(INFO) << "Function [SetRecentYield] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetRecentYield] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * SETAUTOLOADMODE)(unsigned short);
		SETAUTOLOADMODE SetAutoLoadMode = (SETAUTOLOADMODE)GetProcAddress(hDll, "SetAutoLoadMode");
		if (NULL == SetAutoLoadMode)
		{
			LOG(INFO) << "Function [SetAutoLoadMode] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetAutoLoadMode] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * LOADFILE)(int);
		LOADFILE LoadFile = (LOADFILE)GetProcAddress(hDll, "LoadFile");
		if (NULL == LoadFile)
		{
			LOG(INFO) << "Function [LoadFile] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [LoadFile] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * UPDATEQAPARAMINFO)(int, unsigned short, unsigned short, unsigned short, int, int, int, double);
		UPDATEQAPARAMINFO UpdateQAParamInfo = (UPDATEQAPARAMINFO)GetProcAddress(hDll, "UpdateQAParamInfo");
		if (NULL == UpdateQAParamInfo)
		{
			LOG(INFO) << "Function [UpdateQAParamInfo] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [UpdateQAParamInfo] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		//disable new lot
		typedef int(_stdcall * DISABLENEWLOTFUNCTION)();
		DISABLENEWLOTFUNCTION DisableNewLotFunction = (DISABLENEWLOTFUNCTION)GetProcAddress(hDll, "DisableNewLotFunction");
		if (NULL == DisableNewLotFunction)
		{
			LOG(INFO) << "Function [DisableNewLotFunction] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [DisableNewLotFunction] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		//test DATA output path set
		typedef int(_stdcall * SETDATALOGPATH)(char*);
		SETDATALOGPATH SetDatalogPath = (SETDATALOGPATH)GetProcAddress(hDll, "SetDatalogPath");
		if (NULL == SetDatalogPath)
		{
			LOG(INFO) << "Function [SetDatalogPath] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetDatalogPath] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * SETSUMSAMEDATALOG)(byte);
		SETSUMSAMEDATALOG SetSumSameDatalog = (SETSUMSAMEDATALOG)GetProcAddress(hDll, "SetSumSameDatalog");
		if (NULL == SetSumSameDatalog)
		{
			LOG(INFO) << "Function [SetSumSameDatalog] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [SetSumSameDatalog] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * AUTOEXPORTSET)(int);
		AUTOEXPORTSET AutoExportSet = (AUTOEXPORTSET)GetProcAddress(hDll, "AutoExportSet");
		if (NULL == AutoExportSet)
		{
			LOG(INFO) << "Function [AutoExportSet] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [AutoExportSet] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		typedef int(_stdcall * ISTESTUIOPEN)(int);
		ISTESTUIOPEN IsTestuiOpen = (ISTESTUIOPEN)GetProcAddress(hDll, "IsTestuiOpen");
		if (NULL == IsTestuiOpen)
		{
			LOG(INFO) << "Function [IsTestuiOpen] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [IsTestuiOpen] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		/*typedef int(_stdcall * GETEXPORTRESULT)();
		GETEXPORTRESULT GetExportResult = (GETEXPORTRESULT)GetProcAddress(hDll, "GetExportResult");
		if (NULL == GetExportResult)
		{
			LOG(INFO) << "Function [GetExportResult] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [GetExportResult] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}*/
		typedef int(_stdcall * GETEXPORTRESULTWITHOUTTIME)(int);
		GETEXPORTRESULTWITHOUTTIME GetExportResultWithOuttime = (GETEXPORTRESULTWITHOUTTIME)GetProcAddress(hDll, "GetExportResultWithOuttime");
		if (NULL == GetExportResultWithOuttime)
		{
			LOG(INFO) << "Function [GetExportResultWithOuttime] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [GetExportResultWithOuttime] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		/*typedef int(_stdcall * GETEXPORTRESULTWITHMDBVERIFY)(char*);
		GETEXPORTRESULTWITHMDBVERIFY GetExportResultWithMdbVerify = (GETEXPORTRESULTWITHMDBVERIFY)GetProcAddress(hDll, "GetExportResultWithMdbVerify");
		if (NULL == GetExportResultWithMdbVerify)
		{
			LOG(INFO) << "Function [GetExportResultWithMdbVerify] is not exist!";
			MessageBox(GetForegroundWindow(), "Function [GetExportResultWithMdbVerify] is not exist", "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}*/

		SwitchUser("oper", "oper");
		CreateNewLotFile();
		//add RunCard Info auto_setup_acco
		AppendStdfLotMsg("LotID", 0, cMirRecord.LOT_ID, 1, 1);
		AppendStdfLotMsg("PGS_NAM", 4, (char*)configCondFile.Program.c_str(), 1, 0);
		AppendStdfLotMsg("CLOT_ID", 6, cMirRecord.SBLOT_ID, 1, 0);
		AppendStdfLotMsg("OPER_NAM", 7, cMirRecord.OPER_NAM, 1, 0);
		AppendStdfLotMsg("TEST_COD", 31, (char*)testerID.c_str(), 1, 0);
		AppendStdfLotMsg("TST_TEMP", 11, cMirRecord.TST_TEMP, 1, 0);
		AppendStdfLotMsg("FLOW_ID", 23, cMirRecord.FLOW_ID, 1, 0);
		//AppendStdfLotMsg("MODE_COD", 30, "", 1, 0);
		AppendStdfLotMsg("RTST_COD", 31, (char*)RTInfo.c_str(), 1, 0);
		AppendStdfLotMsg("HIFIX_ID", 56, (char*)configCondFile.curr_hifix.c_str(), 1, 0);
		//test PGS input , check that PSG is correc
		if (_access(fmt::format("{0}/{1}", programPath, configCondFile.Program).c_str(), 0) == 0)
		{
			bool ret = RemoveDir((char*)fmt::format("{0}/{1}", programPath, configCondFile.Program).c_str());
			fmt::print("{}", ret);
		}
		fmt::print("正在进行下载解压程序到本地！！！！");

		// NO TPIR
		inputFile = fmt::format("\\\\192.168.202.6/data1/ANX_PGM/{4}_PGM/{0}/{1}/{2}/SWR/{3}.zip"
			, configCondFile.owner, configCondFile.prodid, configCondFile.step, configCondFile.Program, testerType);
		outputFile = programPath;

		if (!ExistsFile(inputFile)) {
			inputFile = fmt::format("\\\\192.168.202.6/data1/ANX_PGM/{4}_PGM/{0}/{1}/{2}/ECN/{3}.zip"
				, configCondFile.owner, configCondFile.externalno, configCondFile.step, configCondFile.Program, testerType);
		}

		// TPIR change
		/*if (configCondFile.IsSWR == "Y") {
			inputFile = fmt::format("\\\\192.168.202.6/data1/ANX_PGM/{4}_PGM/{0}/{1}/{2}/SWR/{3}.zip"
				, configCondFile.owner, configCondFile.externalno, configCondFile.step, configCondFile.Program, testerType);
		} else {
			inputFile = fmt::format("\\\\192.168.202.6/data1/ANX_PGM/{4}_PGM/{0}/{1}/{2}/ECN/{3}.zip"
				, configCondFile.owner, configCondFile.externalno, configCondFile.step, configCondFile.Program, testerType);
		}
		outputFile = programPath;*/

		cmd = fmt::format("7z.exe x {} -o{}/", inputFile, outputFile);
		system((char*)cmd.c_str());

		strPGS = fmt::format("{0}/{1}/{1}.PGS", programPath, configCondFile.Program);
		//strPGS = fmt::format("D:/TEMP_PGM/8200/{0}/{0}.PGS", configCondFile.Program);
		//strPGS = fmt::format("D:/8200/{0}/{0}.PGS", configCondFile.Program);
		Sleep(1000);
		std::string ctime = CurrentTime("%Y%m%d_%H%M%S");
		SetDatalogFileName((char*)fmt::format("[PGS_NAM]-[LotID]-[FLOW_ID]-[RTST_COD]-[TEST_COD]-[HIFIX_ID]-{}", ctime).c_str());
		DatalogFileName = fmt::format("{}-{}-{}-{}-{}-{}-{}", configCondFile.Program.c_str(), cMirRecord.LOT_ID, cMirRecord.FLOW_ID, RTInfo, testerID, configCondFile.curr_hifix.c_str(), ctime);
		fmt::print("DatalogFileName:{}\n", DatalogFileName);

		SetCurrentStation(0);
		int nRet = RunTestui();
		if (nRet != 0)
		{
			LOG(INFO) << fmt::format("Run TestUI failed[{}]!", nRet);
			MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Run TestUI failed[{}]", nRet).c_str(), "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		nRet = EnableQATest();
		if (nRet != 0)
		{
			LOG(INFO) << fmt::format("Enable QA test failed[{}]!", nRet);
			MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Enable QA test failed[{}]", nRet).c_str(), "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		nRet = SetPgsFileName((char *)strPGS.c_str());//including path
		if (nRet != 0)
		{
			MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Set file name failed[{}]", nRet).c_str(), "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		if (NorR == "RT")
		{
			SPLEnable = 0;
		}
		nRet = SetRecentYield(SPLEnable, SampleCount, Yield);
		//MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("{}", SPLEnable).c_str(), "Run Error", 0);
		if (nRet != 0)
		{
			MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Set SetRecentYield failed[{}]", nRet).c_str(), "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}

		SetAutoLoadMode(1);
		ShowWindow(GetConsoleWindow(), SW_HIDE);	//命令行隐藏
		nRet = LoadFile(30000);
		if (nRet != 0)
		{
			LOG(INFO) << fmt::format("Load file failed[{}]!", nRet);
			MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Load file failed[{}]", nRet).c_str(), "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		RemoveDir((char*)(fmt::format("C:/AccoTest/STS8200CROSS/DPAT_TEST_DATA/_{}", configCondFile.Program)).c_str());
		nRet = UpdateQAParamInfo(0, curQAEnable, 1, curQAType, curSetQANumber, curMaxQANumber, curLotNumber, curDefaultPassRate);
		if (nRet != 0)
		{
			LOG(INFO) << fmt::format("Set QA param failed[{}]!", nRet);
			MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Set QA param failed[{}]", nRet).c_str(), "Run Error", 0);
			FreeLibrary(hDll);
			return;
		}
		DisableNewLotFunction();
		//SetDatalogPath("D:/test/DataLog");
		SetDatalogPath((char*)datalogPath.c_str());
		//test SUM output
		SetSumSameDatalog(0);
		AutoExportSet(0x26);
		while (1) {
			int result1 = IsTestuiOpen(0);
			while (1) {
				int result1 = IsTestuiOpen(0);
				while (result1 == 0) {
					/*if (NorR == "DPAT") {
						return;
					}*/
					int result2 = IsTestuiOpen(0);
					if (result2 == -2) {
						Sleep(3000);
						system("cls");
						//ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);	//命令行显示
						if (!ExistsFile(fmt::format("{0}/{1}.mdb", datalogPath, DatalogFileName))) {
							return;
						}
						while (!ExistsFile(fmt::format("{0}/{1}.STD", datalogPath, DatalogFileName)) &&
							!ExistsFile(fmt::format("{0}/{1}.csv", datalogPath, DatalogFileName)) &&
							!ExistsFile(fmt::format("{0}/{1}_sum.csv", datalogPath, DatalogFileName))) {
							Sleep(1000);
						}
						
						std::string QAFilePath = fmt::format("{0}/{1}(QA).csv", datalogPath, DatalogFileName);
						if (QA != false) {
							int sleep_count = 0;
							while (!ExistsFile(QAFilePath)) {
								if (sleep_count > 300){
									MessageBox(GetForegroundWindow(), "文件上传异常", "Run", 0);
									return;
								}
								Sleep(1000);
								sleep_count += 1;
							}
						}

						if (GetExportResultWithOuttime(300000) == 0) {
							Sleep(10000);
						}
						
						/*if (GetExportResult() == 0) {
							MessageBox(GetForegroundWindow(), "数据转换 Successful  111", "数据转换", 0);
							break;
						}*/
						/*if (GetExportResultWithMdbVerify((char*)fmt::format("{}.mdb", DatalogFileName).c_str()) == 0) {
							MessageBox(GetForegroundWindow(), "数据转换 Successful  333", "数据转换", 0);
							break;
						}*/
						
						if (_access(fmt::format("{0}/{1}", programPath, configCondFile.Program).c_str(), 0) == 0)
						{
							RemoveDir((char*)fmt::format("{0}/{1}", programPath, configCondFile.Program).c_str());
						}
						fmt::print("TEST UI 程序结束!!!!!!!!!\n");
						fmt::print("正在进行文件压缩上传!!!!!!\n");
						if (QA == false) {
							inputFile = fmt::format("{0}/{1}.csv {0}/{1}.mdb {0}/{1}.STD {0}/{1}_sum.csv"
								, datalogPath, DatalogFileName);
						}
						else {
							inputFile = fmt::format("{0}/{1}.csv {0}/{1}.mdb {0}/{1}.STD {0}/{1}_sum.csv {0}/{1}(QA).csv"
								, datalogPath, DatalogFileName);
						}
						outputFile = fmt::format("\\\\192.168.202.166\\ft_server_data\\{0}\\Datalog\\", configCondFile.owner);

						std::string temp_str = "";
						while (true) {
							std::string tempFile = fmt::format("D:\\TEMP_DATALOG\\{0}.7z", DatalogFileName);
							cmd = fmt::format("7z.exe a {} {} -y", tempFile, inputFile);
							cmdPopen(cmd);
							cmd = fmt::format("7z.exe t {}", tempFile);
							temp_str = cmdPopen(cmd);
							std::cout << "*************************************************\n";
							std::cout << temp_str << "\n";
							std::string::size_type idx;
							idx = temp_str.find("Everything is Ok");
							if (idx != std::string::npos) {
								if (QA != false) {
									idx = temp_str.find("Files: 5");
								}
								else {
									idx = temp_str.find("Files: 4");
								}
								if (idx != std::string::npos) {
									cmd = fmt::format("move /y {} {}", tempFile, outputFile);
									std::cout << cmd << "\n";
									cmdPopen(cmd);
									break;
								}
							} 
							else {
								system("cls");
								MessageBox(GetForegroundWindow(), "文件上传异常，重新打包文件上传", "Run", 0);
							}
						}
						// LoadSummaryReportforAC
						std::string summaryFile = "\\\\192.168.202.166/ft_server_data/AC/Datalog/" + DatalogFileName + ".7z";
						ResponseData responseData = LoadSummaryReportforAC(summaryFile, configCondFile.lotid, configCondFile.step);
						if (responseData.response.find("SUCCESS") == std::string::npos)
						{
							MessageBox(GetForegroundWindow(), "SBL数据上传失败，请联系PE处理", "Run", 0);
							return;
						}
						std::cout << responseData.response + "\n";

						MessageBox(GetForegroundWindow(), "文件上传结束，可以进行后续操作", "Run", 0);
						//el::Helpers::uninstallPreRollOutCallback();
						return;
					}
					Sleep(500);
				}
			}
		}
	}
	else MessageBox(GetForegroundWindow(), (LPCSTR)fmt::format("Load library [{}]failed!", pcAccoDll).c_str(), "Run Error", 0);
}