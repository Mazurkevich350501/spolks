#pragma once

#include <vector>

using namespace std;

class CommandParser
{
private:
	vector<string> Commands;

public:
	void setMessage(string message);
	string getCommand();
	string getParam(int index);
	int getParamsCount();
};