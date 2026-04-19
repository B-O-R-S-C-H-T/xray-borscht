#pragma once

#include <variant>

#include "../ECore/ImGui/IM_Window.h"

class ESceneCustomOTool;
class CCustomObject;

class IM_ObjectListWnd : public IM_Window {
public:
	static IM_ObjectListWnd* instance;

	IM_ObjectListWnd() { instance = this; }

	~IM_ObjectListWnd() { instance = NULL; }

public:
	void Render() override;
	void RefreshList();

private:
	int	 m_filterTools = 0;
	int	 m_showOptions = 0;
	char m_filter[100] = {};

	bool m_needRefresh = true;

	u32	 m_lastSceneObjCount = 0;

	struct Object {
		CCustomObject* Obj;
		bool		   Selected;
	};
	struct Group {
		ESceneCustomOTool*	Tool;
		bool				Opened;
		std::vector<Object> Objects;
	};
	std::vector<Group>				 m_items;
	void							 refresh();
	void							 clearAllSel();
	void							 showSelected(bool vis);

	bool							 m_needRecalcVis = false;
	std::vector<std::pair<int, int>> m_vis;
	void							 recalcVis();

	int								 getTopSelInGroup(int group);
	int								 getBottomSelInGroup(int group);
};
