#include "Command.h"

std::string execCommand(const std::string cmd, int& out_exitStatus)
{
	out_exitStatus = 0;
	auto pPipe = _popen(cmd.c_str(), "r");
	if (pPipe == nullptr)
	{
		throw std::runtime_error("Cannot open pipe");
	}

	std::array<char, 256> buffer;

	std::string result;

	while (!std::feof(pPipe))
	{
		auto bytes = std::fread(buffer.data(), 1, buffer.size(), pPipe);
		result.append(buffer.data(), bytes);
	}

	auto rc = _pclose(pPipe);

	/*if (WIFEXITED(rc))
	{
		out_exitStatus = WEXITSTATUS(rc);
	}*/

	return result;
}


//ÓÃ·¨
//int exitStatus = 0;
//auto result = execCommand("java -version", exitStatus);