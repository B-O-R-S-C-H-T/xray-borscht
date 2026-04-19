#include "stdafx.h"

#include "FSColorWrapper.h"

typedef bool(WINAPI* FSColorPickerDoModalProc)(unsigned int* currentColor,
											   const bool	 currentColorIsDefault,
											   unsigned int* originalColor,
											   const bool	 originalColorIsDefault,
											   const int	 initialExpansionState);

static HMODULE					hFSColorDLL			  = nullptr;
static FSColorPickerDoModalProc pFSColorPickerDoModal = nullptr;

static bool LoadFSColorDLL() {
	if (hFSColorDLL)
		return true;

	hFSColorDLL = LoadLibraryA("ColorPicker.dll");
	if (!hFSColorDLL) {
		DWORD err = GetLastError();
		ELog.Msg(mtError, "Cannot load ColorPicker.dll!");

		switch (err) {
			case 126:
				ELog.Msg(mtError, "\tDLL not found.");
				break;
			case 193:
				ELog.Msg(mtError, "\tWrong architecture: 32-bit DLL cannot be loaded into 64-bit process.");
				break;
			case 5:
				ELog.Msg(mtError, "\tAccess denied. Check file permissions.");
				break;
			case 998:
				ELog.Msg(mtError, "\tInvalid access to memory location (bad DLL).");
				break;
			default:
				ELog.Msg(mtError, "\tUnknown LoadLibrary error.");
		}
		return false;
	}

	pFSColorPickerDoModal = (FSColorPickerDoModalProc)GetProcAddress(hFSColorDLL, "FSColorPickerDoModal");
	if (!pFSColorPickerDoModal) {
		DWORD err = GetLastError();
		ELog.Msg(mtError, "Cannot find function 'FSColorPickerDoModal' in ColorPicker.dll. Error: %u", err);
		FreeLibrary(hFSColorDLL);
		hFSColorDLL = nullptr;
		return false;
	}

//	ELog.Msg(mtInformation, "ColorPicker.dll loaded successfully.");
	return true;
}

ECORE_API bool FSColorPicker(u32* color) {
	VERIFY(color);

	if (!LoadFSColorDLL())
		return false;

	u32 originalColor = *color;

	if (pFSColorPickerDoModal(color, true, &originalColor, true, FSCOLOR_EXPANDED)) {
		return true;
	}
	return false;
}
