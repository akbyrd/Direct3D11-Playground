#pragma once

#define throw_assert(x, msg)                     \
do { if ( !(x) )                                 \
{                                                \
	throw AssertionException(#x, msg,            \
	          __FILE__, __LINE__, __FUNCTION__); \
} } while (0)

class AssertionException : public std::exception
{
public:
	AssertionException(std::string, std::string, const char*, DWORD, const char*);

	virtual const char* what() const;

private:
	std::string message;
};