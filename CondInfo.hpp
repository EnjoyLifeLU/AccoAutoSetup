// CondInfo.hpp
// head-source
#pragma once
#include <boost\algorithm\string.hpp>
#include <boost\algorithm\string\trim.hpp>
#include <boost\algorithm\string\trim_all.hpp>
#include "read_config.hpp"
#include <fstream>
#include <string>
#include <vector>

// test
#include <stdio.h>
#include <iostream>

struct ConfigCond {
	std::string SYSmsg;
	std::string owner;
	std::string lotid;
	std::string clotid;
	std::string prodid;
	std::string step;
	std::string tform;
	std::string testerModel;
	std::string hifixed;
	std::string ISEQC;
	std::string BINDefine;
	std::string RETESTBIN;
	std::string Program;
	std::string Programver;
	std::string pgmpath;
	std::string tpirno;
	std::string oper;
	std::string temperature;
	std::string OPID;
	std::string Soaktime;
	std::string GUARD;
	std::string usoaktime;
	std::string Packagetype;
	std::string Packagewidth;
	std::string leadno;
	std::string socket_name;
	std::string socket_path;
	std::string discard_str;
	std::string externalno;
	std::string curr_hifix;
	std::string IsSWR;
};

ConfigCond configCondFile;

bool LoadCond(std::string condFilePath) {
	try {
		std::ifstream condFile;
		condFile.open(condFilePath);
		char szbuff[1024] = { 0 };
		std::vector<std::string> split_str;
		std::string buff;
		condFile.getline(szbuff, 1024);
		boost::split(split_str, szbuff, boost::is_any_of(" "),
			boost::token_compress_on);
		for (int i = split_str.size(); i <= 25; i++) {
			split_str.push_back("");
		}
		configCondFile.SYSmsg = split_str[0];
		configCondFile.owner = split_str[1];
		configCondFile.lotid = split_str[2];
		configCondFile.clotid = split_str[3];
		configCondFile.prodid = split_str[4];
		configCondFile.step = split_str[5];
		configCondFile.tform = split_str[6];
		configCondFile.testerModel = split_str[7];
		configCondFile.hifixed = split_str[8];
		configCondFile.ISEQC = split_str[9];
		configCondFile.BINDefine = split_str[10];
		configCondFile.RETESTBIN = split_str[11];
		configCondFile.Program = split_str[12];
		configCondFile.Programver = split_str[13];
		configCondFile.pgmpath = split_str[14];
		configCondFile.tpirno = split_str[15];
		configCondFile.oper = split_str[16];
		configCondFile.temperature = split_str[17];
		configCondFile.OPID = split_str[18];
		configCondFile.Soaktime = split_str[19];
		configCondFile.GUARD = split_str[20];
		configCondFile.usoaktime = split_str[21];
		configCondFile.Packagetype = split_str[22];
		configCondFile.Packagewidth = split_str[23];
		configCondFile.leadno = split_str[24];
		configCondFile.externalno = split_str[25];

		// add 2022.09.19
		/*if (configCondFile.owner == "AC")
			configCondFile.prodid = configCondFile.prodid.substr(
			0, configCondFile.prodid.find_last_of("}") + 1);*/

		return true;
	}
	catch (std::exception e) {
		return false;
	}
}

bool LoadCond2(std::string condFilePath) {
	try {
		std::ifstream condFile;
		condFile.open(condFilePath);
		char szbuff[2048] = { 0 };
		std::string buff;
		condFile.getline(szbuff, 2048);
		buff = szbuff;

		configCondFile.SYSmsg = buff.substr(0, 30);
		configCondFile.owner = buff.substr(30, 10);
		configCondFile.lotid = buff.substr(40, 50);
		configCondFile.clotid = buff.substr(90, 50);
		configCondFile.prodid = buff.substr(140, 50);
		configCondFile.step = buff.substr(190, 25);
		configCondFile.tform = buff.substr(215, 25);
		configCondFile.testerModel = buff.substr(240, 25);
		configCondFile.hifixed = buff.substr(265, 50);
		configCondFile.ISEQC = buff.substr(315, 25);
		configCondFile.BINDefine = buff.substr(340, 25);
		configCondFile.RETESTBIN = buff.substr(365, 25);
		configCondFile.Program = buff.substr(390, 100);
		configCondFile.Programver = buff.substr(490, 25);
		configCondFile.pgmpath = buff.substr(515, 100);
		configCondFile.tpirno = buff.substr(615, 50);
		configCondFile.oper = buff.substr(665, 25);
		configCondFile.temperature = buff.substr(690, 25);
		configCondFile.OPID = buff.substr(715, 25);
		configCondFile.Soaktime = buff.substr(740, 25);
		configCondFile.GUARD = buff.substr(765, 25);
		configCondFile.usoaktime = buff.substr(790, 25);
		configCondFile.Packagetype = buff.substr(815, 25);
		configCondFile.Packagewidth = buff.substr(840, 25);
		configCondFile.leadno = buff.substr(865, 25);
		configCondFile.socket_name = buff.substr(890, 25);
		configCondFile.socket_path = buff.substr(915, 25);
		configCondFile.discard_str = buff.substr(940, 25);
		configCondFile.externalno = buff.substr(965, 50);
		configCondFile.curr_hifix = buff.substr(1015, 25);
		configCondFile.IsSWR = buff.substr(1040, 5);

		configCondFile.SYSmsg = boost::algorithm::trim_copy(configCondFile.SYSmsg);
		configCondFile.owner = boost::algorithm::trim_copy(configCondFile.owner);
		configCondFile.lotid = boost::algorithm::trim_copy(configCondFile.lotid);
		configCondFile.clotid = boost::algorithm::trim_copy(configCondFile.clotid);
		configCondFile.prodid = boost::algorithm::trim_copy(configCondFile.prodid);
		configCondFile.step = boost::algorithm::trim_copy(configCondFile.step);
		configCondFile.tform = boost::algorithm::trim_copy(configCondFile.tform);
		configCondFile.testerModel = boost::algorithm::trim_copy(configCondFile.testerModel);
		configCondFile.hifixed = boost::algorithm::trim_copy(configCondFile.hifixed);
		configCondFile.ISEQC = boost::algorithm::trim_copy(configCondFile.ISEQC);
		configCondFile.BINDefine = boost::algorithm::trim_copy(configCondFile.BINDefine);
		configCondFile.RETESTBIN = boost::algorithm::trim_copy(configCondFile.RETESTBIN);
		configCondFile.Program = boost::algorithm::trim_copy(configCondFile.Program);
		configCondFile.Programver = boost::algorithm::trim_copy(configCondFile.Programver);
		configCondFile.pgmpath = boost::algorithm::trim_copy(configCondFile.pgmpath);
		configCondFile.tpirno = boost::algorithm::trim_copy(configCondFile.tpirno);
		configCondFile.oper = boost::algorithm::trim_copy(configCondFile.oper);
		configCondFile.temperature = boost::algorithm::trim_copy(configCondFile.temperature);
		configCondFile.OPID = boost::algorithm::trim_copy(configCondFile.OPID);
		configCondFile.Soaktime = boost::algorithm::trim_copy(configCondFile.Soaktime);
		configCondFile.GUARD = boost::algorithm::trim_copy(configCondFile.GUARD);
		configCondFile.usoaktime = boost::algorithm::trim_copy(configCondFile.usoaktime);
		configCondFile.Packagetype = boost::algorithm::trim_copy(configCondFile.Packagetype);
		configCondFile.Packagewidth = boost::algorithm::trim_copy(configCondFile.Packagewidth);
		configCondFile.leadno = boost::algorithm::trim_copy(configCondFile.leadno);
		configCondFile.socket_name = boost::algorithm::trim_copy(configCondFile.socket_name);
		configCondFile.socket_path = boost::algorithm::trim_copy(configCondFile.socket_path);
		configCondFile.discard_str = boost::algorithm::trim_copy(configCondFile.discard_str);
		configCondFile.externalno = boost::algorithm::trim_copy(configCondFile.externalno);
		configCondFile.curr_hifix = boost::algorithm::trim_copy(configCondFile.curr_hifix);
		configCondFile.IsSWR = boost::algorithm::trim_copy(configCondFile.IsSWR);

		//// add 2022.09.19
		//if (configCondFile.owner == "AC")
		//	configCondFile.prodid = configCondFile.prodid.substr(
		//	0, configCondFile.prodid.find_last_of("}") + 1);

		//// add 2022.11.17
		//if (testerID_flag == 1)
		//	configCondFile.externalno = configCondFile.prodid;

		return true;
	}
	catch (std::exception e) {
		return false;
	}
}