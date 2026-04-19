#pragma once

#include "xrCore/xrCore.h"

extern "C"
{
	__declspec(dllimport) bool WINAPI FSColorPickerDoModal(unsigned int* currentColor,
														   const bool	 currentColorIsDefault,
														   unsigned int* originalColor,
														   const bool	 originalColorIsDefault,
														   const int	 initialExpansionState);
}

#define FSCOLOR_DEFAULT	  0
#define FSCOLOR_EXPANDED  1
#define FSCOLOR_COLLAPSED 3

ECORE_API bool FSColorPicker(u32* color);
