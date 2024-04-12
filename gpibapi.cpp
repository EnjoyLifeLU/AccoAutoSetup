#include "gpibapi.h"

#pragma comment(lib, "ni4882.lib")
#define BDINDEX 0 // Board Index
#define PRIMARY_ADDR_OF_HANDLER 3 // Primary address of Device
#define NO_SECONDARY_ADDR 0 // Secondary address of Device
#define TIMEOUT T10s // Timeout value = 10 seconds
#define EOTMODE 1 // Enable the END message
#define EOSMODE 0 // Disable the EOS mode

int Dev;
char ValueStr[256];

BOOL GPIBErrorDetected(int Dev, char* ErrorMsg)
{
	char ErrorStr[1024];
	const char ErrorMnemonic[21][5] = { "EDVR", "ECIC", "ENOL",
		"EADR", "EARG", "ESAC",
		"EABO", "ENEB", "EDMA", "",
		"EOIP", "ECAP", "EFSO", "",
		"EBUS", "ESTB", "ESRQ", "",
		"", "", "ETAB" };
	if (ibsta & ERR) {
		sprintf(ErrorStr, "IEEE-Error : %s\n\nibsta = 0x%x iberr = %d (%s)\n", ErrorMsg, ibsta, iberr, ErrorMnemonic[iberr]);
		printf("%s\n", ErrorStr);
		return TRUE; // Signal Error to application
	}
	else
		return FALSE; // No Error -> continue
}

char* GPIBQuery(char* SendString)
{
	Dev = ibdev(BDINDEX, PRIMARY_ADDR_OF_HANDLER,
		NO_SECONDARY_ADDR, TIMEOUT, EOTMODE, EOSMODE);
	if (GPIBErrorDetected(Dev, "Unable to connect to handler via IEEE")) { 
		return NULL;
	}
	// send out device clear
	ibclr(Dev);
	Sleep(10);
	if (GPIBErrorDetected(Dev, "Unable to clear device")) return NULL;
	// read back the LOTID
	sprintf(ValueStr, SendString);
	ibwrt(Dev, ValueStr, strlen(ValueStr));
	Sleep(10);
	if (GPIBErrorDetected(Dev, "Write of handler ID request string failed")) return NULL;
	ibrd(Dev, ValueStr, 40);
	Sleep(10);
	if (GPIBErrorDetected(Dev, "Reading of handler ID failed")) return NULL;
	ValueStr[ibcntl] = 0;
	//printf("Returned Handler ID:%s.", ValueStr);
	ibonl(Dev, 0);
	return ValueStr;
}