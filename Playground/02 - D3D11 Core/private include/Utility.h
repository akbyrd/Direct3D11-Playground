#include "stdafx.h"

#define RELEASE_COM(x) { Utility::ReleaseCOM((IUnknown**) (x)); }

namespace Utility
{
	void ReleaseCOM(IUnknown**);
}