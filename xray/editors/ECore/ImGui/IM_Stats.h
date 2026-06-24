#pragma once

#include "IM_Window.h"

class ECORE_API IM_Stats : public IM_Window {
private:
	float m_saved_x = 60.0f;
	float m_saved_y = 100.0f;
	float m_saved_w = 300.0f;
	float m_saved_h = 200.0f;

	float m_last_x	= 0.0f;
	float m_last_y	= 0.0f;
	float m_last_w	= 0.0f;
	float m_last_h	= 0.0f;

public:
	virtual void OnAdd() override;
	virtual void OnRemove() override;
	virtual void Render() override;
};

extern ECORE_API IM_Stats imStats;
