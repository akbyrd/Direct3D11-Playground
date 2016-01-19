#pragma once

#include <Windows.h>

#include <exception>
#include <string>

#define throw_logged(msg)                        \
do { if ( !(msg) )                               \
{                                                \
	__debugbreak();                              \
	throw LoggedException(msg,                   \
	          __FILE__, __LINE__, __FUNCTION__); \
} } while (0)

class LoggedException : public std::exception
{
public:
	LoggedException(std::wstring, const char*, DWORD, const char*);

	virtual const char* what() const;

protected:
	LoggedException();
	std::string message;
};