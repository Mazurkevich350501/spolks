#include "CommandParser.h"
#include <sstream>

void CommandParser::setMessage(string message)
{
	stringstream messageStream(message);
	string buf;
	Commands.clear();
	while (messageStream >> buf)
		Commands.push_back(buf);
}

string CommandParser::getCommand()
{
	return Commands.capacity() != 0 ? Commands[0] : "";
}

string CommandParser::getParam(int index)
{
	if (index > 0 && index < Commands.capacity())
	{
		return Commands[index];
	}
	else
	{
		return "";
	}
}

int CommandParser::getParamsCount()
{
	return Commands.size() - 1;
}