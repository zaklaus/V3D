#include <EASTL/allocator.h>

#include <stdio.h>
#include <wchar.h>

// EASTL expects us to define these, see allocator.h line 194
void* operator new[](size_t size, const char* /*pName*/, int /*flags*/,
                     unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
    return ::new char[size];
}

void* operator new[](size_t size, size_t alignment, size_t /*alignmentOffset*/,
                     const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
    // this allocator doesn't support alignment
    EASTL_ASSERT(alignment <= 8);
    return ::new char[size];
}

///////////////////////////////////////////////////////////////////////////////
// Required by EASTL.
//
#if !EASTL_EASTDC_VSNPRINTF
	int Vsnprintf8(char* pDestination, size_t n, const char*  pFormat, va_list arguments)
	{
		return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
	}

	int Vsnprintf16(char16_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments)
	{
		return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
	}

	int Vsnprintf32(char32_t* pDestination, size_t n, const char32_t* pFormat, va_list arguments)
	{
		return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
	}

	#if defined(EA_CHAR8_UNIQUE) && EA_CHAR8_UNIQUE
		int Vsnprintf8(char8_t* pDestination, size_t n, const char8_t*  pFormat, va_list arguments)
		{
			return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
		}
	#endif

	#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
		int VsnprintfW(wchar_t* pDestination, size_t n, const wchar_t* pFormat, va_list arguments)
		{
			return EA::StdC::Vsnprintf(pDestination, n, pFormat, arguments);
		}
	#endif
#endif
