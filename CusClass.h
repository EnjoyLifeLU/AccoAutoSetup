#include <iostream>
//#include <string>
using namespace std;
class cutString
{
public:
	cutString(const char* pStr = "")//���캯��
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
		cout << "String()���� " << endl;
	}

	cutString(const cutString& s) //�������캯��
		:_pStr(new char[strlen(s._pStr) + 1])
	{
		cout << "String()�������� " << endl;
		strcpy(_pStr, s._pStr);
	}

	cutString& operator=(const cutString& s)//��ֵ����������
	{
		if (this != &s)
		{
			cout << "cutString& =()��ֵ " << endl;
			//������1�����ָ���  �������ʧ�ܣ���ԭ���Ķ��󲻻����Ӱ��
			char* pTmp = new char[strlen(s._pStr) + 1];//�����¿ռ�
			strcpy(pTmp, s._pStr);//����Ԫ��
			delete[] _pStr;//�ͷžɿռ�
			_pStr = pTmp;//ʹ���¿ռ�

			//��������2��һ��ʼ�ͷŵ�ǰ�ռ䣬�������ʧ�ܣ���ǰ�Ŀռ�Ҳ��������
			/*delete[] _pStr;
			_pStr = new char[strlen(s._pStr) + 1];
			strcpy(_pStr, s._pStr);*/
		}
		return *this;
	}

	~cutString() //��������
	{
		if (_pStr)
		{
			cout << "~cutString()���� " << endl;
			delete[] _pStr;
			_pStr = NULL;
		}
	}

private:
	char* _pStr;
};