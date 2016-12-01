#pragma once
#define PACKAGE_LENGTH 1472
#define HEAD_LENGTH 6
#define MAX_DATA_LENGTH (PACKAGE_LENGTH - HEAD_LENGTH)

struct Package
{
	int Number;
	unsigned short int Length;
	char Data[MAX_DATA_LENGTH];
};
