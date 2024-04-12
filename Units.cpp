#include "Units.h"

void remove_dir(std::string path) {
	boost::filesystem::remove_all(path);
}

bool CopyDirectory(const std::string &strSourceDir, const std::string &strDestDir)
{
	boost::filesystem::recursive_directory_iterator end; //���ñ���������־����recursive_directory_iterator����ѭ���ı���Ŀ¼
	boost::system::error_code ec;
	for (boost::filesystem::recursive_directory_iterator pos(strSourceDir); pos != end; ++pos)
	{
		//ɸѡ����
		//���˵�Ŀ¼����Ŀ¼Ϊ�յ����
		if (boost::filesystem::is_directory(*pos))
			continue;
		std::string strAppPath = boost::filesystem::path(*pos).string();
		std::string strRestorePath;
		//replace_first_copy��algorithm/stringͷ�ļ��У���strAppPath�в���strSourceDir�ַ������ҵ�����strDestDir�滻���滻����ַ���������һ�������������
		boost::replace_first_copy(std::back_inserter(strRestorePath), strAppPath, strSourceDir, strDestDir);
		if (!boost::filesystem::exists(boost::filesystem::path(strRestorePath).parent_path()))
		{
			boost::filesystem::create_directories(boost::filesystem::path(strRestorePath).parent_path(), ec);
		}
		boost::filesystem::copy_file(strAppPath, strRestorePath, boost::filesystem::copy_option::overwrite_if_exists, ec);
	}
	if (ec)
	{
		return false;
	}
	return true;
}

bool RemoveDir(const char* szFileDir)
{
	std::string strDir = szFileDir;
	if (strDir.at(strDir.length() - 1) != '\\')
		strDir += '\\';
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((strDir + "*.*").c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_stricmp(wfd.cFileName, ".") != 0 &&
				_stricmp(wfd.cFileName, "..") != 0)
				RemoveDir((strDir + wfd.cFileName).c_str());
		}
		else
		{
			DWORD FileAttribute = GetFileAttributes((strDir + wfd.cFileName).c_str());
			if (FileAttribute == INVALID_FILE_ATTRIBUTES)
			{
				return FALSE;
			}
			else if (FileAttribute & FILE_ATTRIBUTE_READONLY)
			{
				SetFileAttributes((strDir + wfd.cFileName).c_str(), FILE_ATTRIBUTE_NORMAL);
			}
			DeleteFile((strDir + wfd.cFileName).c_str());
		}
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind);
	RemoveDirectory(szFileDir);
	return true;
}

bool ExistsFile(const std::string& name) 
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool FindProcess(std::string strProcessName, DWORD& nPid)
{

	TCHAR tszProcess[64] = { 0 };
	lstrcpy(tszProcess, _T(strProcessName.c_str()));
	//���ҽ���
	STARTUPINFO st;
	PROCESS_INFORMATION pi;
	PROCESSENTRY32 ps;
	HANDLE hSnapshot;
	memset(&st, 0, sizeof(STARTUPINFO));
	st.cb = sizeof(STARTUPINFO);
	memset(&ps, 0, sizeof(PROCESSENTRY32));
	ps.dwSize = sizeof(PROCESSENTRY32);
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	// ��������  
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return false;
	if (!Process32First(hSnapshot, &ps))
		return false;
	do {
		if (lstrcmp(ps.szExeFile, tszProcess) == 0)
		{
			//�ҵ��ƶ��ĳ���
			nPid = ps.th32ProcessID;
			CloseHandle(hSnapshot);
			printf("�ҵ�����: %s\n", tszProcess);
			return true;
			//getchar();
			//return dwPid;
		}
	} while (Process32Next(hSnapshot, &ps));
	CloseHandle(hSnapshot);
	return false;
}

bool KillProcess(DWORD dwPid)
{
	printf("Kill����Pid = %d\n", dwPid); getchar();
	//�رս���
	HANDLE killHandle = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION |   // Required by Alpha
		PROCESS_CREATE_THREAD |   // For CreateRemoteThread
		PROCESS_VM_OPERATION |   // For VirtualAllocEx/VirtualFreeEx
		PROCESS_VM_WRITE,             // For WriteProcessMemory);
		FALSE, dwPid);
	if (killHandle == NULL)
		return false;
	TerminateProcess(killHandle, 0);
	return true;
}

bool StartPrcess(std::string strProcessName)
{
	TCHAR tszProcess[64] = { 0 };
	lstrcpy(tszProcess, _T(strProcessName.c_str()));
	//��������
	SHELLEXECUTEINFO shellInfo;
	memset(&shellInfo, 0, sizeof(SHELLEXECUTEINFO));
	shellInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	shellInfo.fMask = NULL;
	shellInfo.hwnd = NULL;
	shellInfo.lpVerb = NULL;
	shellInfo.lpFile = tszProcess;						// ִ�еĳ�����(����·��)
	shellInfo.lpParameters = NULL;
	shellInfo.lpDirectory = NULL;
	shellInfo.nShow = SW_MINIMIZE;						//SW_SHOWNORMAL ȫ����ʾ�������
	shellInfo.hInstApp = NULL;
	printf("�����Զ�������.... \n");
	ShellExecuteEx(&shellInfo);
	return true;
}

//void FocusChanged(bool isfocus)
//{
//	HIMC hIMC_ = NULL;
//	if (isfocus)
//	{
//		//focus on		
//		hIMC_ = ::ImmGetContext(GetConsoleWindow());
//		if (hIMC_)
//		{
//			ImmAssociateContext(GetConsoleWindow(), nullptr);
//			ImmReleaseContext(GetConsoleWindow(), hIMC_);
//		}
//		return;
//	}
//	else
//	{
//		//focus out
//		if (hIMC_)
//		{
//			ImmDestroyContext(hIMC_);
//			hIMC_ = nullptr;
//		}
//		hIMC_ = ImmCreateContext();
//		if (hIMC_)
//		{
//			ImmAssociateContext(GetConsoleWindow(), hIMC_);
//			ImmReleaseContext(GetConsoleWindow(), hIMC_);
//		}
//	}
//}

void CapsLock()
{
	short s = GetKeyState(VK_CAPITAL);
	BYTE btKeyState[256];
	::GetKeyboardState((LPBYTE)&btKeyState);
	if (!(btKeyState[VK_CAPITAL] & 1))
	{
		keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}

void trim(std::string &s)
{
	int index = 0;
	if (!s.empty())
	{
		while ((index = s.find(' ', index)) != std::string::npos)
		{
			s.erase(index, 1);
		}
	}
}


std::string cmdPopen(const std::string& cmdLine) 
{
	char buffer[1024] = { '\0' };
	FILE* pf = NULL;
	pf = _popen(cmdLine.c_str(), "r");
	if (NULL == pf) {
		printf("open pipe failed\n");
		return std::string("");
	}
	std::string ret;
	while (fgets(buffer, sizeof(buffer), pf)) {
		ret += buffer;
	}
	_pclose(pf);
	return ret;
}