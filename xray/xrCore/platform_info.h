///////////////////////////////////////////////////////////////////////////////
// platform_info.h
// Сборщик информации о платформе и операционной системе
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

struct platform_info {
	std::string os_name;
	uint32_t	os_build			= 0;
	size_t		physical_ram_mb		= 0;
	int			process_cores		= 0;
	int			total_logical_cores = 0;

	void		initialize();
};

extern platform_info g_platform_info;

void log_platform_info();
