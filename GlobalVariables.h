//GlobalVariables.h
//head
#pragma once

struct SdrRecord //声明结构体类型
{
	char* HAND_TYP; // Handler or prober type
	char* HAND_ID; // Handler or prober ID
	char* CARD_TYP; // Probe card type
	char* CARD_ID; // Probe card ID
	char* LOAD_TYP; // Load board type
	char* LOAD_ID; // Load board ID
	char* DIB_TYP; // DIB board type
	char* DIB_ID; // DIB board ID
	char* CABL_TYP; // Interface cable type
	char* CABL_ID; // Interface cable ID
	char* CONT_TYP; // Handler contactor type
	char* CONT_ID; // Handler contactor ID
	char* LASR_TYP; // Laser type
	char* LASR_ID; // Laser ID
	char* EXTR_TYP; // 附加设备类型
	char* EXTR_ID; // 附加设备ID号
};
//SdrRecord cSdrRecord;


struct MirRecord
{
	char MODE_COD; // 测试模式（如prod,dev）
	char RTST_COD; // 重测代码 Y =重测 N = 没重测 0 - 9 = 重测次数
	char PROT_COD; // Data protection code
	unsigned short BURN_TIM = 65535; // Burn_in time
	char CMOD_COD; // Command mode code
	char* LOT_ID = ""; //Lot Id
	char* PART_TYP = ""; //Part type(or Product Id) 即 DEVICENAME
	char* NODE_NAM = ""; //Name of node that generated data
	char* TSTR_TYP = ""; //Tester type（测试机类型）
	char* JOB_NAM = ""; //任务名（测试程序名）
	char* JOB_REV = ""; //测试程序版本号
	char* SBLOT_ID = ""; //Sublot ID
	char* OPER_NAM = ""; //操作员名称或ID号
	char* EXEC_TYP = ""; //Tester executive software type （测试软件类型）
	char* EXEC_VER = ""; //测试软件版本
	char* TEST_COD = ""; //Test phase or step code，相当于WORK CENTER
	char* TST_TEMP = ""; //测试温度
	char* USER_TXT = ""; //Generic user text
	char* AUX_FILE = ""; //辅助数据文件名
	char* PKG_TYP = ""; //Package type
	char* FAMILY_ID = ""; //Product family ID （产品系列ID）
	char* DATE_COD = ""; //Date code
	char* FACIL_ID = ""; //Test facility（测试设备ID）
	char* FLOOR_ID = ""; //Test floor ID（测试台ID）
	char* PROC_ID = ""; //Fabrication process ID
	char* OPER_FRQ = ""; //Operation frequency or step
	char* SPEC_NAM = ""; //Test specification name
	char* SPEC_VER = ""; //Test specification version number
	char* FLOW_ID = ""; //Test flow ID
	char* SETUP_ID = ""; //Test setup ID
	char* DSGN_REV = ""; //Device design revision
	char* ENG_ID = ""; //Engineering lot ID
	char* ROM_COD = ""; //ROM code ID
	char* SERL_NUM = ""; //Tester serial number
	char* SUPR_NAM = ""; //Supervisor name or ID
};
//MirRecord cMirRecord;

struct wrrRecord
{
	char* WAFER_ID = ""; // Wafer ID length byte = 0
	char* FABWF_ID = ""; // Fab wafer ID length byte = 0
	char* FRAME_ID = ""; // Wafer frame ID length byte = 0
	char* MASK_ID = ""; // Wafer mask ID length byte = 0
	char* USR_DESC = ""; // Wafer描述（user） length byte = 0
	char* EXC_DESC = ""; // Wafer描述（exc） length byte = 0
};
//wrrRecord cWrrRecord;

struct UMSAutoLoad
{
	char* PSGPATH = "";// Unimos Auto Load program path
	char* PSGNAME = "";// Unimos Auto Load program name
	char* PGSFILE = "";// .pgs file is the program file
};
UMSAutoLoad cUMSAutoLoad;

enum RunMode
{
	PROD_MODE = 1,
	ENG_MODE = 2
};
//RunMode RunModes;