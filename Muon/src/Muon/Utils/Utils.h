/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Utility functions
----------------------------------------------*/
#ifndef MUON_UTILS_H
#define MUON_UTILS_H

#include <Muon/Core/Core.h>

namespace Muon
{
	inline void MUON_API Print(const char* str);
	inline void MUON_API Print(const wchar_t* str);
	inline void MUON_API Printf(const char* format, ...);
	inline void MUON_API Printf(const wchar_t* format, ...);

	// Aligns a size or memory offset to be a multiple of alignment
	inline UINT MUON_API AlignToBoundary(UINT size, UINT alignment);
}

#endif