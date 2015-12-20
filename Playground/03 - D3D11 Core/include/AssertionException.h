#pragma once

#define throw_assert(x, msg)                     \
do { if ( !(x) )                                 \
{                                                \
	__debugbreak();                              \
	throw AssertionException(L#x, msg,           \
	          __FILE__, __LINE__, __FUNCTION__); \
} } while (0)

class AssertionException : public std::exception
{
public:
	AssertionException(std::wstring, std::wstring, const char*, DWORD, const char*);

	virtual const char* what() const;

private:
	std::string message;
};