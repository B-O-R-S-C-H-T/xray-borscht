#pragma once

#include "..\ECore\ImGui\IM_Window.h"

class IM_MainMenu : public IM_Window {
public:
	virtual void Render();
};

class IM_About : public IM_Window {
public:
	bool m_open;
	bool visited_source;
	bool visited_updates;
	IM_About() : m_open(false), visited_source(false), visited_updates(false) {}
	void Open() { m_open = true; }
	virtual void Render();
};

extern IM_MainMenu imMainMenu;
