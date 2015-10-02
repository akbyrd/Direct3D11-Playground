#include "stdafx.h"

#define LOG_ERROR(hr) D3D11Utility::LogError(hr, WIDE(__FILE__), __LINE__, WIDE(__FUNCTION__))

static class D3D11Utility final
{
public:
	static bool LogError(HRESULT, wchar_t*, long, wchar_t*);
};