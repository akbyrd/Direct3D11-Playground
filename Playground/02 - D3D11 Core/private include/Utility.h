#pragma once

#include "stdafx.h"

using namespace DirectX;

#define RELEASE_COM(x) { Utility::ReleaseCOM((IUnknown**) &(x)); }

namespace Utility
{
	void ReleaseCOM(IUnknown**);
}

namespace Color
{
	XMGLOBALCONST XMVECTORF32 Red   = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue  = { 0.0f, 0.0f, 1.0f, 1.0f };
}