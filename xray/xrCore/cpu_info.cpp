///////////////////////////////////////////////////////////////////////////////
// cpu_info.cpp
// Сборщик информации о процессоре и его возможностях
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "cpu_info.h"

#include <intrin.h>
#include <vector>

static bool check_xcr0_avx() {
	unsigned long long xcr0 = _xgetbv(0);
	return (xcr0 & 0x6) == 0x6;
}

static bool check_xcr0_avx512() {
	unsigned long long xcr0 = _xgetbv(0);
	return (xcr0 & 0xE6) == 0xE6;
}

const cpu_info& get_cpu_info() {
	static cpu_info instance;
	return instance;
}

void cpu_info_initialize() {
	cpu_info& info = const_cast<cpu_info&>(get_cpu_info());

	if (info.m_is_initialized)
		return;

	int regs[4] = {};

	// Vendor ID
	__cpuid(regs, 0);
	memcpy(info.m_vendor, &regs[1], 4);
	memcpy(info.m_vendor + 4, &regs[3], 4);
	memcpy(info.m_vendor + 8, &regs[2], 4);
	info.m_vendor[12] = '\0';

	// Brand
	__cpuid(regs, 0x80000002);
	memcpy(info.m_brand, regs, sizeof(regs));
	__cpuid(regs, 0x80000003);
	memcpy(info.m_brand + 16, regs, sizeof(regs));
	__cpuid(regs, 0x80000004);
	memcpy(info.m_brand + 32, regs, sizeof(regs));
	info.m_brand[48] = '\0';

	// Строка дополняется пробелами до 48 байт, отрезаем лишнее
	{
		char* end = info.m_brand + strlen(info.m_brand);
		while (end > info.m_brand && isspace((unsigned char)*(end - 1))) {
			*(--end) = '\0';
		}
	}

	// Family/Model/Stepping
	__cpuid(regs, 1);
	info.m_stepping = regs[0] & 0xF;
	info.m_model    = ((regs[0] >> 4) & 0xF) | (((regs[0] >> 16) & 0xF) << 4);
	info.m_family   = ((regs[0] >> 8) & 0xF) | (((regs[0] >> 20) & 0xFF) << 4);

	// Feature flags
	auto& f = info.m_features.bits;

	f[0]  = (regs[3] & (1 << 23)) != 0;   // MMX
	f[2]  = (regs[3] & (1 << 25)) != 0;   // SSE
	f[3]  = (regs[3] & (1 << 26)) != 0;   // SSE2
	f[4]  = (regs[2] & (1 << 0))  != 0;   // SSE3
	f[5]  = (regs[2] & (1 << 3))  != 0;   // MWAIT
	f[6]  = (regs[2] & (1 << 9))  != 0;   // SSSE3
	f[7]  = (regs[2] & (1 << 19)) != 0;   // SSE4.1
	f[8]  = (regs[2] & (1 << 20)) != 0;   // SSE4.2
	f[10] = (regs[2] & (1 << 28)) != 0;   // AVX

	__cpuid(regs, 0x80000001);
	f[1]  = (regs[3] & (1 << 31)) != 0;   // 3DNow!
	f[9]  = (regs[2] & (1 << 6))  != 0;   // SSE4a

	__cpuidex(regs, 7, 0);
	f[11] = (regs[1] & (1 << 5))  != 0;   // AVX2
	f[19] = (regs[1] & (1 << 3))  != 0;   // BMI1
	f[20] = (regs[1] & (1 << 8))  != 0;   // BMI2
	f[12] = (regs[1] & (1 << 16)) != 0;   // AVX512F
	f[13] = (regs[1] & (1 << 31)) != 0;   // AVX512VL
	f[14] = (regs[1] & (1 << 30)) != 0;   // AVX512BW
	f[15] = (regs[1] & (1 << 17)) != 0;   // AVX512DQ

	__cpuidex(regs, 7, 1);
	f[18] = (regs[0] & (1 << 19)) != 0;   // AVX10
	f[16] = (regs[0] & (1 << 11)) != 0;   // AVX512VNNI
	f[17] = (regs[0] & (1 << 21)) != 0;   // AVX512IFMA

	// Проверка поддержки AVX со стороны ОС
	if (f[10] && !check_xcr0_avx()) {
		f[10] = f[11] = false;
	}
	if (f[12] && !check_xcr0_avx512()) {
		f[12] = f[13] = f[14] = f[15] = f[16] = f[17] = f[18] = false;
	}

	using PGetLogicalProcessorInformationEx = BOOL(WINAPI*)(LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD);
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	if (hKernel32) {
		auto func = (PGetLogicalProcessorInformationEx)GetProcAddress(hKernel32, "GetLogicalProcessorInformationEx");

		if (func) {
			DWORD buffer_size = 0;
			func(RelationAll, nullptr, &buffer_size);

			if (buffer_size > 0) {
				std::vector<BYTE> buffer(buffer_size);
				auto*			  procInfo = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data());

				if (func(RelationAll, procInfo, &buffer_size)) {
					BYTE* ptr = buffer.data();
					BYTE* end = ptr + buffer_size;

					while (ptr < end) {
						auto* current = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(ptr);

						if (current->Relationship == RelationProcessorCore) {
							info.m_physical_cores++;
							if (current->Processor.Flags & LTP_PC_SMT)
								info.m_logical_cores += 2;
							else
								info.m_logical_cores++;
						} else if (current->Relationship == RelationCache) {
							const auto& cache = current->Cache;
							if (cache.Level == 1)   info.m_l1_cache_size += cache.CacheSize;
							if (cache.Level == 2)   info.m_l2_cache_size += cache.CacheSize;
							if (cache.Level == 3)   info.m_l3_cache_size += cache.CacheSize;
						}

						ptr += current->Size;
					}
				}
			}
		}
	}

	if (info.m_logical_cores == 0)  info.m_logical_cores  = 1;
	if (info.m_physical_cores == 0) info.m_physical_cores = info.m_logical_cores;

	info.m_is_initialized = true;
}

void cpu_info::clear_features() {
	m_features.clear();
}

std::string cpu_info::get_hardware_features_string() const {
	std::string s = "RDTSC";

	if (m_features.has_mmx())       s += ", MMX";
	if (m_features.has_sse())       s += ", SSE";
	if (m_features.has_sse2())      s += ", SSE2";
	if (m_features.has_sse3())      s += ", SSE3";
	if (m_features.has_ssse3())     s += ", SSSE3";
	if (m_features.has_sse41())     s += ", SSE4.1";
	if (m_features.has_sse42())     s += ", SSE4.2";
	if (m_features.has_sse4a())     s += ", SSE4a";
	if (m_features.has_avx())       s += ", AVX";
	if (m_features.has_avx2())      s += ", AVX2";
	if (m_features.has_avx512f())   s += ", AVX-512F";
	if (m_features.has_avx10())     s += ", AVX10";
	if (m_features.has_bmi1())      s += ", BMI1";
	if (m_features.has_bmi2())      s += ", BMI2";

	return s;
}

std::string cpu_info::get_os_enabled_features_string() const {
	std::string s = "RDTSC";

	if (m_features.has_mmx())       s += ", MMX";
	if (m_features.has_sse())       s += ", SSE";
	if (m_features.has_sse2())      s += ", SSE2";
	if (m_features.has_sse3())      s += ", SSE3";
	if (m_features.has_ssse3())     s += ", SSSE3";
	if (m_features.has_sse41())     s += ", SSE4.1";
	if (m_features.has_sse42())     s += ", SSE4.2";
	if (m_features.has_sse4a())     s += ", SSE4a";

	if (m_features.has_avx())       s += ", AVX";
	if (m_features.has_avx2())      s += ", AVX2";
	if (m_features.has_avx512f())   s += ", AVX-512F";
	if (m_features.has_avx10())     s += ", AVX10";

	if (m_features.has_bmi1())      s += ", BMI1";
	if (m_features.has_bmi2())      s += ", BMI2";

	return s;
}
