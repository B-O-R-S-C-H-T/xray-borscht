#include "stdafx.h"
#pragma hdrstop

#include "EStats.h"

#include <Layers/xrRender/HW.h>
#include <xrEngine/igame_persistent.h>

CStats::CStats() {
	fFPS				  = 30.f;
	fRFPS				  = 30.f;
	fTPS				  = 0;

	dwShader_Codes		  = 0;
	dwShader_Textures	  = 0;
	dwShader_Matrices	  = 0;
	dwShader_Constants	  = 0;

	dwSND_Played		  = 0;
	dwSND_Allocated		  = 0;

	dwTotalLight		  = 0;
	dwLightInScene		  = 0;

	dwLevelSelFaceCount	  = 0;
	dwLevelSelVertexCount = 0;

	RenderDUMP_DT_Count	  = 0;
}

CStats::~CStats() {}

void CStats::Show() {
	// Stop timers
	{
		RenderTOTAL.FrameEnd		();
		RenderCALC.FrameEnd			();

        RenderDUMP_SKIN.FrameEnd	();

		Animation.FrameEnd			();
		Input.FrameEnd				();
		clRAY.FrameEnd				();
		clBOX.FrameEnd				();
        clFRUSTUM.FrameEnd			();

        RenderDUMP_RT.FrameEnd		();

		RenderDUMP_DT_VIS.FrameEnd	();
		RenderDUMP_DT_Render.FrameEnd();
		RenderDUMP_DT_Cache.FrameEnd();
        TEST0.FrameEnd				();
        TEST1.FrameEnd				();
        TEST2.FrameEnd				();
        TEST3.FrameEnd				();
	}

	// calc FPS & TPS
	CBackend::_stats& DPS = RCache.stat;
	if (Device.fTimeDelta > EPS_S) {
		float fps  = 1.f / Device.fTimeDelta;
		float fOne = 0.3f;
		float fInv = 1.f - fOne;
		fFPS	   = fInv * fFPS + fOne * fps;

		if (RenderTOTAL.result > EPS_S) {
			fTPS  = fInv * fTPS + fOne * float(DPS.polys) / (RenderTOTAL.result * 1000.f);
			fRFPS = fInv * fRFPS + fOne * 1000.f / RenderTOTAL.result;
		}
	}

}
