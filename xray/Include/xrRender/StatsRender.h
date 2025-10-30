#ifndef	StatsRender_included
#define	StatsRender_included
#pragma once

class CGameFont;
class IRenderDeviceRender;

class IStatsRender
{
public:
	virtual ~IStatsRender() { ; }
	virtual void Copy(IStatsRender& _in) = 0;
	virtual void OutData1()              = 0;
	virtual void OutData2()              = 0;
	virtual void OutData3()              = 0;
	virtual void OutData4()              = 0;
	virtual void GuardVerts()            = 0;
	virtual void GuardDrawCalls()        = 0;
};

#endif	//	StatsRender_included