#include "str_op.h"

std::vector<std::string> split_str(const std::string& str, const std::string& delim)
{
	std::vector<std::string> res;
	if ("" == str) return res;
	char* strs = new char[str.length() + 1];
	strcpy(strs, str.c_str());

	char* d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char* p = strtok(strs, d);
	while (p) {
		std::string s = p;
		res.push_back(s);
		p = strtok(NULL, d);
	}
	return res;
}

void replace_all(std::string& old_str, std::string& new_str, std::string const& old_value, std::string const& new_value)
{
	new_str = old_str;
	std::string::size_type pos = new_str.find(old_value), t_size = old_value.size(), r_size = new_value.size();
	while (pos != std::string::npos) { // found   
		new_str.replace(pos, t_size, new_value);
		pos = new_str.find(old_value, pos + r_size);
	}
}

void unique_vec_str(std::vector<std::string>& str)
{
	sort(str.begin(), str.end());
	str.erase(unique(str.begin(), str.end()), str.end());
}

std::string UnicodeToUTF8(const std::wstring& wstr)
{
	std::string curLocale = setlocale(LC_ALL, NULL);    //"C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = wstr.c_str();
	size_t _Dsize = 2 * wstr.size() + 1;
	char* _Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	std::string result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

std::wstring UTF8ToUnicode(const std::string& str)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = str.c_str();
	size_t _Dsize = str.size() + 1;
	wchar_t* _Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	std::wstring result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

char* CalcAlpha(char alpha, int num) {
	int c = (int(alpha) + num);
	char re[2];
	re[0] = char(c);
	re[1] = '\0';
	return re;
}