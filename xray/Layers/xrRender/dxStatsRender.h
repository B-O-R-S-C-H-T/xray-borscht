#ifndef	dxStatsRender_included
#define	dxStatsRender_included
#pragma once

#include "..\..\Include\xrRender\StatsRender.h"

class dxStatsRender : public IStatsRender
{
public:
	virtual void Copy(IStatsRender &_in);

	virtual void OutData1();
	virtual void OutData2();
	virtual void OutData3();
	virtual void OutData4();
	virtual void GuardVerts();
	virtual void GuardDrawCalls();

private:
};

#endif	//	dxStatsRender_included