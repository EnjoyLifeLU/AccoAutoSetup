#pragma once
//#include "Units.h"

#include <string>
#include <windows.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <ShellAPI.h>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>


bool RemoveDir(const char* szFileDir);
bool ExistsFile(const std::string& name);
bool FindProcess(std::string strProcessName, DWORD& nPid);
bool KillProcess(DWORD dwPid);
bool StartPrcess(std::string strProcessName);
//void FocusChanged(bool isfocus);
void CapsLock();
void trim(std::string &s);
void remove_dir(std::string path);
bool CopyDirectory(const std::string &strSourceDir, const std::string &strDestDir);
std::string cmdPopen(const std::string& cmdLine);
