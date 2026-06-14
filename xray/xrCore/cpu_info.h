///////////////////////////////////////////////////////////////////////////////
// cpu_info.h
// Сборщик информации о процессоре и его возможностях
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <bitset>
#include <string>

struct cpu_features {
	std::bitset<32> bits;

	bool has_mmx()        const { return bits[0];  }
	bool has_3dnow()      const { return bits[1];  }
	bool has_sse()        const { return bits[2];  }
	bool has_sse2()       const { return bits[3];  }
	bool has_sse3()       const { return bits[4];  }
	bool has_mwait()      const { return bits[5];  }
	bool has_ssse3()      const { return bits[6];  }
	bool has_sse41()      const { return bits[7];  }
	bool has_sse42()      const { return bits[8];  }
	bool has_sse4a()      const { return bits[9];  }
	bool has_avx()        const { return bits[10]; }
	bool has_avx2()       const { return bits[11]; }
	bool has_avx512f()    const { return bits[12]; }
	bool has_avx512vl()   const { return bits[13]; }
	bool has_avx512bw()   const { return bits[14]; }
	bool has_avx512dq()   const { return bits[15]; }
	bool has_avx512vnni() const { return bits[16]; }
	bool has_avx512ifma() const { return bits[17]; }
	bool has_avx10()      const { return bits[18]; }
	bool has_bmi1()       const { return bits[19]; }
	bool has_bmi2()       const { return bits[20]; }

	void clear() { bits.reset(); }
};

struct cpu_info {
	char        m_vendor[13]   = {};
	char        m_brand[49]    = {};
	uint32_t    m_family       = 0;
	uint32_t    m_model        = 0;
	uint32_t    m_stepping     = 0;

	uint32_t    m_logical_cores  = 0;
	uint32_t    m_physical_cores = 0;

	size_t      m_l1_cache_size = 0;
	size_t      m_l2_cache_size = 0;
	size_t      m_l3_cache_size = 0;

	cpu_features m_features;

	bool        m_is_initialized = false;

	// для запуска без SIMD
	void clear_features();

	std::string get_hardware_features_string() const;
	std::string get_os_enabled_features_string() const;
};

XRCORE_API const cpu_info& get_cpu_info();
XRCORE_API void			   cpu_info_initialize();
