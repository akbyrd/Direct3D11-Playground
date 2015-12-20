#include "stdafx.h"
#include "AssertionException.h"

using namespace std;

AssertionException::AssertionException(
	wstring     expression,
	wstring     message,
	const char* file,
	DWORD       line,
	const char* function)
{
	Logging::LogAssert(expression, message, file, line, function);
}

const char* AssertionException::what() const
{
	return message.c_str();
}