/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Utility functions
----------------------------------------------*/

#include <Muon.h>
#include <Muon/Utils/Utils.h>

namespace Muon
{
	inline void MUON_API Print(const char* str)
	{
		OutputDebugStringA(str);
	}

	inline void MUON_API Print(const wchar_t* str)
	{
		OutputDebugString(str);
	}

	inline void MUON_API Printf(const char* format, ...)
	{
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buffer, 256, format, ap);
		va_end(ap);
		Print(buffer);
	}

	inline void MUON_API Printf(const wchar_t* format, ...)
	{
		wchar_t buffer[256];
		va_list ap;
		va_start(ap, format);
		vswprintf(buffer, 256, format, ap);
		va_end(ap);
		Print(buffer);
	}

	inline UINT MUON_API AlignToBoundary(UINT size, UINT alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}
}