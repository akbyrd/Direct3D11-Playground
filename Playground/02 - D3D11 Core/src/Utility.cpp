#include "stdafx.h"

namespace Utility
{
	void ReleaseCOM(IUnknown** ppCOMObject)
	{
		if ( *ppCOMObject )
		{
			(*ppCOMObject)->Release();
			(*ppCOMObject) = nullptr;
		}
	}
}