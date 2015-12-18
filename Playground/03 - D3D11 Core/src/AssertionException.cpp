#include "stdafx.h"
#include "AssertionException.h"

using namespace std;

AssertionException::AssertionException(
	string      expression,
	string      message,
	const char* file,
	DWORD       line,
	const char* function)
{
	ostringstream msgStream;
	msgStream << "Assertion failed: " << expression << endl;
	          //<< 
}

const char* AssertionException::what() const
{
	return message.c_str();
}