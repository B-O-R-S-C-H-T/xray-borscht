//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "UI_ShaderMain.h"
#include "UI_ShaderTools.h"
#ifndef NO_VCL
#include "topbar.h"
#include "leftbar.h"
#endif
#include "../../Layers/xrRender/D3DUtils.h"
#ifndef NO_VCL
#include "bottombar.h"
#include "main.h"
#endif

#include "StatusBar.h"

//---------------------------------------------------------------------------
CShaderMain*&	PUI=(CShaderMain*&)UI;
//---------------------------------------------------------------------------

CShaderMain::CShaderMain()
{
    EPrefs			= xr_new<CCustomPreferences>();
}
//---------------------------------------------------------------------------

CShaderMain::~CShaderMain()
{
    xr_delete		(EPrefs);
}
//---------------------------------------------------------------------------

CCommandVar CShaderTool::CommandSave(CCommandVar p1, CCommandVar p2)
{
    Save			(0,0);
    ExecCommand		(COMMAND_UPDATE_CAPTION);
    return TRUE;
}
CCommandVar CShaderTool::CommandSaveBackup(CCommandVar p1, CCommandVar p2)
{
    ExecCommand		(COMMAND_SAVE);
    return TRUE;
}
CCommandVar CShaderTool::CommandReload(CCommandVar p1, CCommandVar p2)
{
    Reload			();
    ExecCommand		(COMMAND_UPDATE_CAPTION);
    return TRUE;
}
CCommandVar CShaderTool::CommandClear(CCommandVar p1, CCommandVar p2)
{
    Device.m_Camera.Reset();
    ExecCommand		(COMMAND_UPDATE_CAPTION);
    return TRUE;
}
CCommandVar CShaderTool::CommandUpdateList(CCommandVar p1, CCommandVar p2)
{
	UpdateList		();
    return TRUE;
}
CCommandVar CommandRefreshUIBar(CCommandVar p1, CCommandVar p2)
{
#ifndef NO_VCL
    fraTopBar->RefreshBar	();
    fraLeftBar->RefreshBar	();
    fraBottomBar->RefreshBar();
#endif
    return TRUE;
}
CCommandVar CommandRestoreUIBar(CCommandVar p1, CCommandVar p2)
{
#ifndef NO_VCL
    fraTopBar->fsStorage->RestoreFormPlacement();
    fraLeftBar->fsStorage->RestoreFormPlacement();
    fraBottomBar->fsStorage->RestoreFormPlacement();
#endif
    return TRUE;
}
CCommandVar CommandSaveUIBar(CCommandVar p1, CCommandVar p2)
{
#ifndef NO_VCL
    fraTopBar->fsStorage->SaveFormPlacement();
    fraLeftBar->fsStorage->SaveFormPlacement();
    fraBottomBar->fsStorage->SaveFormPlacement();
#endif
    return TRUE;
}
CCommandVar CommandUpdateToolBar(CCommandVar p1, CCommandVar p2)
{
#ifndef NO_VCL
    fraLeftBar->UpdateBar();
#endif
    return TRUE;
}
CCommandVar CommandUpdateCaption(CCommandVar p1, CCommandVar p2)
{
#ifndef NO_VCL
    frmMain->UpdateCaption();
#endif
    return TRUE;
}

void CShaderMain::RegisterCommands()
{
	inherited::RegisterCommands();
    // tools
	REGISTER_CMD_CE	(COMMAND_SAVE,           	"File\\Save", 	STools,CShaderTool::CommandSave,true);
	REGISTER_CMD_C	(COMMAND_SAVE_BACKUP,	 	STools,CShaderTool::CommandSaveBackup);
	REGISTER_CMD_CE	(COMMAND_LOAD,			 	"File\\Reload",	STools,CShaderTool::CommandReload,true);
	REGISTER_CMD_CE	(COMMAND_CLEAR,			 	"File\\Clear", 	STools,CShaderTool::CommandClear,true);
    REGISTER_CMD_CE	(COMMAND_UPDATE_LIST,	 	"Update List",	STools,CShaderTool::CommandUpdateList,true);
    REGISTER_CMD_S	(COMMAND_REFRESH_UI_BAR, 	CommandRefreshUIBar);
    REGISTER_CMD_S	(COMMAND_RESTORE_UI_BAR,  	CommandRestoreUIBar);
    REGISTER_CMD_S	(COMMAND_SAVE_UI_BAR,     	CommandSaveUIBar);
	REGISTER_CMD_S	(COMMAND_UPDATE_TOOLBAR,  	CommandUpdateToolBar);
    REGISTER_CMD_S	(COMMAND_UPDATE_CAPTION,	CommandUpdateCaption);
}

LPCSTR CShaderMain::GetCaption()
{
	return (LPSTR)STools->CurrentToolsName();// "shaders&materials";
}           

bool CShaderMain::ApplyShortCut(WORD Key, TShiftState Shift)
{
    return inherited::ApplyShortCut(Key,Shift);
}
//---------------------------------------------------------------------------

bool CShaderMain::ApplyGlobalShortCut(WORD Key, TShiftState Shift)
{
    return inherited::ApplyGlobalShortCut(Key,Shift);
}
//---------------------------------------------------------------------------

void CShaderMain::RealUpdateScene()
{
	inherited::RealUpdateScene	();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Common
//---------------------------------------------------------------------------
void CShaderMain::ResetStatus()
{
	VERIFY(m_bReady);
#ifndef NO_VCL
    if (fraBottomBar->paStatus->Caption!=""){
	    fraBottomBar->paStatus->Caption=""; fraBottomBar->paStatus->Repaint();
    }
#endif
    SB_SetStatus("");
    SB_DrawProgress();
}
void CShaderMain::SetStatus(LPCSTR s, bool bOutLog)
{
	VERIFY(m_bReady);
#ifndef NO_VCL
    if (fraBottomBar->paStatus->Caption!=s){
	    fraBottomBar->paStatus->Caption=s; fraBottomBar->paStatus->Repaint();
    	if (bOutLog&&s&&s[0]) ELog.Msg(mtInformation,s);
    }
#else
	if(SB_SetStatus(s ? s : ""));
	{
		SB_DrawProgress();
		if (bOutLog&&s&&s[0]) ELog.Msg(mtInformation,s);
	}
#endif
}
void CShaderMain::ProgressDraw()
{
#ifndef NO_VCL
	fraBottomBar->RedrawBar();
#endif
	SB_DrawProgress();
}
//---------------------------------------------------------------------------
void CShaderMain::OutCameraPos()
{
	if (m_bReady){
        string64 s;
        const Fvector& c 	= Device.m_Camera.GetPosition();
        sprintf(s, "C: %3.1f, %3.1f, %3.1f",c.x,c.y,c.z);
    //	const Fvector& hpb 	= Device.m_Camera.GetHPB();
    //	s.sprintf(" Cam: %3.1f°, %3.1f°, %3.1f°",rad2deg(hpb.y),rad2deg(hpb.x),rad2deg(hpb.z));
#ifndef NO_VCL
        fraBottomBar->paCamera->Caption=s; fraBottomBar->paCamera->Repaint();
#endif
        SB_SetInfo(1, s);
    }
}
//---------------------------------------------------------------------------
void CShaderMain::OutUICursorPos()
{
#ifndef NO_VCL
	VERIFY(fraBottomBar);
#endif
    string64 s; POINT pt;
    GetCursorPos(&pt);
    sprintf(s,"Cur: %d, %d",pt.x,pt.y);
#ifndef NO_VCL
    fraBottomBar->paUICursor->Caption=s; fraBottomBar->paUICursor->Repaint();
#endif
	SB_SetInfo(2, s);
}
//---------------------------------------------------------------------------
void CShaderMain::OutGridSize()
{
#ifndef NO_VCL
	VERIFY(fraBottomBar);
#endif
    string64 s;
    sprintf(s,"Grid: %1.1f",EPrefs->grid_cell_size);
#ifndef NO_VCL
    fraBottomBar->paGridSquareSize->Caption=s; fraBottomBar->paGridSquareSize->Repaint();
#endif
	SB_SetInfo(3, s);
}
//---------------------------------------------------------------------------
void CShaderMain::OutInfo()
{
#ifndef NO_VCL
	fraBottomBar->paSel->Caption = Tools->GetInfo();
#endif
	SB_SetInfo(0, Tools->GetInfo());
}
//---------------------------------------------------------------------------
void CShaderMain::RealQuit()
{
#ifndef NO_VCL
    frmMain->Close();
#endif
}
//---------------------------------------------------------------------------

