#include <iostream>
//#include <string>
using namespace std;
class cutString
{
public:
	cutString(const char* pStr = "")//构造函数
	{
		if (NULL == pStr)
		{
			_pStr = new char[1];
			*_pStr = '\0';
		}
		else
		{
			_pStr = new char[strlen(pStr) + 1];
			strcpy(_pStr, pStr);
		}
		cout << "String()构造 " << endl;
	}

	cutString(const cutString& s) //拷贝构造函数
		:_pStr(new char[strlen(s._pStr) + 1])
	{
		cout << "String()拷贝构造 " << endl;
		strcpy(_pStr, s._pStr);
	}

	cutString& operator=(const cutString& s)//赋值操作符重载
	{
		if (this != &s)
		{
			cout << "cutString& =()赋值 " << endl;
			//———1、这种更好  如果开辟失败，对原来的对象不会造成影响
			char* pTmp = new char[strlen(s._pStr) + 1];//申请新空间
			strcpy(pTmp, s._pStr);//拷贝元素
			delete[] _pStr;//释放旧空间
			_pStr = pTmp;//使用新空间

			//————2、一开始释放当前空间，如果开辟失败，以前的空间也不存在了
			/*delete[] _pStr;
			_pStr = new char[strlen(s._pStr) + 1];
			strcpy(_pStr, s._pStr);*/
		}
		return *this;
	}

	~cutString() //析构函数
	{
		if (_pStr)
		{
			cout << "~cutString()析构 " << endl;
			delete[] _pStr;
			_pStr = NULL;
		}
	}

private:
	char* _pStr;
};