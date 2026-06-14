///////////////////////////////////////////////////////////////////////////////
// platform_info.cpp
// Сборщик информации о платформе и операционной системе
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "platform_info.h"

#include <intrin.h>

platform_info g_platform_info;

void platform_info::initialize() {
	 MEMORYSTATUSEX memory_status = {sizeof(memory_status)};
	 if (GlobalMemoryStatusEx(&memory_status)) {
		 physical_ram_mb = memory_status.ullTotalPhys / (1024ULL * 1024);
	 }

	 DWORD_PTR process_affinity = 0, system_affinity = 0;
	 if (GetProcessAffinityMask(GetCurrentProcess(), &process_affinity, &system_affinity)) {
		 process_cores = 0;
		 for (int i = 0; i < 64; i++) {
			 if (process_affinity & (1ULL << i)) {
				 process_cores++;
			 }
		 }
	 }

	 RTL_OSVERSIONINFOW os_version_info	 = {sizeof(os_version_info)};
	 bool				version_obtained = false;
	 HMODULE			ntdll_module	 = GetModuleHandleW(L"ntdll.dll");
	 if (ntdll_module) {
		 auto rtl_get_version = (LONG(WINAPI*)(PRTL_OSVERSIONINFOW))GetProcAddress(ntdll_module, "RtlGetVersion");
		 if (rtl_get_version && rtl_get_version(&os_version_info) == 0) {
			 version_obtained = true;
		 }
	 }

	 // если кто-то будет запускать под Wine
	 if (!version_obtained) {
		 OSVERSIONINFOEXW os_version_ex = {sizeof(os_version_ex)};
		 if (GetVersionExW((OSVERSIONINFOW*)&os_version_ex)) {
			 os_version_info.dwMajorVersion = os_version_ex.dwMajorVersion;
			 os_version_info.dwMinorVersion = os_version_ex.dwMinorVersion;
			 os_version_info.dwBuildNumber	= os_version_ex.dwBuildNumber;
			 version_obtained				= true;
		 }
	 }

	 if (version_obtained) {
		 os_build = os_version_info.dwBuildNumber;
		 if (os_version_info.dwMajorVersion == 10 && os_version_info.dwBuildNumber >= 22000)	os_name = "Windows 11";
		 else if (os_version_info.dwMajorVersion == 10)											os_name = "Windows 10";
		 else if (os_version_info.dwMajorVersion == 6 && os_version_info.dwMinorVersion == 3)	os_name = "Windows 8.1";
		 else if (os_version_info.dwMajorVersion == 6 && os_version_info.dwMinorVersion == 2)	os_name = "Windows 8";
		 else if (os_version_info.dwMajorVersion == 6 && os_version_info.dwMinorVersion == 1)	os_name = "Windows 7";
		 else																					os_name = "Old Windows";
	 } else {
		 os_name  = "Unknown Windows";
		 os_build = 0;
	 }
}

void log_platform_info() {
	if (g_platform_info.os_name.empty()) {
		g_platform_info.initialize();
	}
	Msg("* System: %s (build %u), %zu MB RAM", g_platform_info.os_name.c_str(), g_platform_info.os_build, g_platform_info.physical_ram_mb);
	if (g_platform_info.process_cores > 0) {
		Msg("* Process affinity: %d/%d cores available", g_platform_info.process_cores, g_platform_info.total_logical_cores);
	}
}
