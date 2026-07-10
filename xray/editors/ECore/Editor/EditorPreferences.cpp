#include "stdafx.h"

#include <imgui.h>

#include "../ImGui/IM_PropertyTree.h"
#include "../LevelEditor/ImGui/IM_Toolbar.h"
#include "ImGui/IM_SVGCanvas.h"
#include "ui_main.h"
#include "ui_toolscustom.h"

SWindowPlacement::SWindowPlacement() {
	SetDefaultSafeState();
}

void SWindowPlacement::SetDefaultSafeState() {
	ZeroMemory(&wp, sizeof(wp));
	wp.length				   = sizeof(WINDOWPLACEMENT);
	wp.flags				   = 0;
	wp.showCmd				   = SW_SHOWNORMAL;
	wp.ptMinPosition.x		   = -1;
	wp.ptMinPosition.y		   = -1;
	wp.ptMaxPosition.x		   = -1;
	wp.ptMaxPosition.y		   = -1;
	wp.rcNormalPosition.left   = 100;
	wp.rcNormalPosition.top	   = 100;
	wp.rcNormalPosition.right  = 1280;
	wp.rcNormalPosition.bottom = 720;
}

void SWindowPlacement::Save(HWND hWnd) {
	if (!hWnd)
		return;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd, &wp);
}

void SWindowPlacement::Restore(HWND hWnd) const {
	if (!hWnd)
		return;
	if (wp.length != sizeof(WINDOWPLACEMENT))
		return;

	WINDOWPLACEMENT temp = wp; // copy
	SetWindowPlacement(hWnd, &temp);
}

void SWindowPlacement::ValidateAndFix() {
	if (wp.length != sizeof(WINDOWPLACEMENT)) {
		SetDefaultSafeState();
		return;
	}

	const LONG minW = 640;
	const LONG minH = 480;

	RECT& rc = wp.rcNormalPosition;

	if (rc.right  - rc.left < minW)     rc.right  = rc.left + minW;
	if (rc.bottom - rc.top  < minH)     rc.bottom = rc.top  + minH;

	if (MonitorFromRect(&rc, MONITOR_DEFAULTTONULL) == NULL) {
		RECT workArea{};
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

		LONG w	  = rc.right - rc.left;
		LONG h	  = rc.bottom - rc.top;

		rc.left	  = workArea.left + (workArea.right - workArea.left - w) / 2;
		rc.top	  = workArea.top + (workArea.bottom - workArea.top - h) / 2;
		rc.right  = rc.left + w;
		rc.bottom = rc.top + h;
	}
}

CCustomPreferences* EPrefs=0;

CCustomPreferences::CCustomPreferences()
{
    // view
    view_np				= 0.1f;
    view_fp				= 1500.f;
    view_fov			= deg2rad(60.f);
    // fog    
    fog_color			= 0x00555555;
    fog_fogness			= 0.9;
    // camera
    cam_fly_speed		= 5.0f;
    cam_fly_alt			= 1.8f;
    cam_sens_rot		= 0.6f;
    cam_sens_move		= 0.6f;
    cam_undo_enabled	= TRUE;
	cam_undo_duration	= 0.3f;
    // tools mouse
    tools_sens_move		= 0.3f;
    tools_sens_rot		= 0.3f;
    tools_sens_scale	= 0.3f;
    // box pick
    bp_lim_depth		= TRUE;
    bp_cull				= TRUE;
    bp_depth_tolerance	= 0.1f;
    // snap
    snap_angle			= deg2rad(5.f);
    snap_move			= 0.1f;
    snap_moveto			= 0.5f;
    // grid
    grid_cell_size		= 1.f;
    grid_cell_count		= 100;
    // scene
    scene_undo_level	= 125;
    scene_recent_count	= 10;
    scene_clear_color	= DEFAULT_CLEARCOLOR;
    // objects
    object_flags.zero	();

    log_default_color   = 0x00E8E8E8;
    log_error_color     = 0x00FFC4C4;
    log_info_color      = 0x00E7FFE6;
    log_confirmation_color = 0x00E7E6FF;

    ui_scale            = 1.0f;

	toolbar_vertical    = true;
	toolbar_icon_scale  = 1.3f;
}

CCustomPreferences::~CCustomPreferences()
{
	for (auto& pair : m_SectionStorages) {
		xr_delete(pair.second);
	}
	m_SectionStorages.clear();
}

void CCustomPreferences::ApplyValues()
{
	Tools->m_MoveSnap		= snap_move;
	Tools->m_MoveSnapTo		= snap_moveto;
	Tools->m_RotateSnapAngle= snap_angle;

    Device.m_Camera.SetViewport(view_np, view_fp, rad2deg(view_fov));
    Tools->SetFog	(fog_color,fog_fogness);

    UI->m_MouseSM	= 0.2f*tools_sens_move*tools_sens_move;
    UI->m_MouseSR	= 0.02f*tools_sens_rot*tools_sens_rot;
    UI->m_MouseSS	= 0.02f*tools_sens_scale*tools_sens_scale;

    Device.m_Camera.SetSensitivity	(cam_sens_move, cam_sens_rot);
    Device.m_Camera.SetFlyParams	(cam_fly_speed, cam_fly_alt);

    ExecCommand		(COMMAND_UPDATE_GRID);

	if (ImGui::GetCurrentContext() != nullptr) {
		ImGui::GetIO().FontGlobalScale = ui_scale;
	}
}

void CCustomPreferences::OnUIScaleApply(PropItem* item) {
	ApplyValues();

	if (ImGui::GetCurrentContext() != nullptr) {
		IM_SVGCanvas::ForceRebuild();
	}
}

void CCustomPreferences::OnToolbarVerticalChange(PropValue* prop) {
	IM_Storage& storage = GetSectionStorage("im_toolbar");
	storage.PutBool("vertical", toolbar_vertical);
}

void CCustomPreferences::OnToolbarIconScaleChange(PropValue* prop) {
	IM_Storage& storage = GetSectionStorage("im_toolbar");
	storage.PutFloat("icon_scale", toolbar_icon_scale);
}

void CCustomPreferences::OnClose	()
{
    ApplyValues	();	
	UI->RemoveIMWindow(IMProps);
	Save();
}

void CheckValidate(ShortcutValue*, const xr_shortcut& new_val, bool& result)
{
    result 					= true;

    if(new_val.similar(xr_shortcut(0, FALSE, FALSE, FALSE))) // empty shortcut is always free
        return;

    if(new_val.key==0) { // key == 0 && ext != 0 - invalid
        result = false;
        return;
    }
	
    ECommandVec& cmds		= GetEditorCommands();
    for (u32 cmd_idx=0; cmd_idx<cmds.size(); cmd_idx++){
        SECommand*& CMD		= cmds[cmd_idx];
        if (CMD&&CMD->editable){
            VERIFY(!CMD->sub_commands.empty());
		    for (u32 sub_cmd_idx=0; sub_cmd_idx<CMD->sub_commands.size(); sub_cmd_idx++){
            SESubCommand*& SUB_CMD = CMD->sub_commands[sub_cmd_idx];
                if (SUB_CMD->shortcut.similar(new_val)){ result = false; return;}
            }
        }
    }
}

void CCustomPreferences::OnKeyboardCommonFileClick(ButtonValue* B, bool& bModif, bool&)
{
    bModif = false;
    xr_string fn;
	switch(B->btn_num){
    case 0:
        if(EFS.GetOpenName("$import$", fn, false, NULL, 6)){
            CInifile* 	I 	= xr_new<CInifile>(fn.c_str(), TRUE, TRUE, TRUE);
		    LoadShortcuts	(I);
            xr_delete		(I);
            #ifndef NO_VCL
            m_ItemProps->RefreshForm();
            #endif
        }
    break;
    case 1:
        if(EFS.GetSaveName("$import$", fn, NULL, 6)){
		    CInifile* 	I 	= xr_new<CInifile>(fn.c_str(), FALSE, TRUE, TRUE);
		    SaveShortcuts	(I);
            xr_delete		(I);
        }
    break;
	}
}

void CCustomPreferences::FillProp(PropItemVec& props)
{
    PHelper().CreateFlag32	(props,"Objects\\Library\\Discard Instance",	&object_flags, 	epoDiscardInstance);
    PHelper().CreateFlag32	(props,"Objects\\Skeleton\\Draw Joints",		&object_flags, 	epoDrawJoints);
    PHelper().CreateFlag32	(props,"Objects\\Skeleton\\Draw Bone Axis",		&object_flags, 	epoDrawBoneAxis);
    PHelper().CreateFlag32	(props,"Objects\\Skeleton\\Draw Bone Names",	&object_flags, 	epoDrawBoneNames);
    PHelper().CreateFlag32	(props,"Objects\\Skeleton\\Draw Bone Shapes",	&object_flags, 	epoDrawBoneShapes);
    PHelper().CreateFlag32	(props,"Objects\\Show\\Hint",					&object_flags, 	epoShowHint);
    PHelper().CreateFlag32	(props,"Objects\\Show\\Pivot",					&object_flags, 	epoDrawPivot);
    PHelper().CreateFlag32	(props,"Objects\\Show\\Animation Path",			&object_flags, 	epoDrawAnimPath);
    PHelper().CreateFlag32	(props,"Objects\\Show\\LOD",					&object_flags, 	epoDrawLOD);
    PHelper().CreateFlag32	(props,"Objects\\Loading\\Deffered Loading RB",	&object_flags, 	epoDeffLoadRB);
    PHelper().CreateFlag32	(props,"Objects\\Loading\\Deffered Loading CF",	&object_flags, 	epoDeffLoadCF);

	PHelper().CreateU32     (props, "Scene\\Recent Files\\Maximum Recent Files",         &scene_recent_count, 0, 25);

	PHelper().CreateU32     (props, "Scene\\Undo Manager\\Undo Queue Size",              &scene_undo_level,   0, 125);
	PHelper().CreateBOOL    (props, "Scene\\Undo Manager\\Enable Camera Interpolation",  &cam_undo_enabled);
	PHelper().CreateFloat   (props, "Scene\\Undo Manager\\Interpolation Duration (sec)", &cam_undo_duration, 0.05f, 2.0f, 0.01f, 2);

	PHelper().CreateFloat	(props,"Scene\\Grid\\Cell Size", 	           	&grid_cell_size,	0.1f,	10.f);
    PHelper().CreateU32		(props,"Scene\\Grid\\Cell Count", 	           	&grid_cell_count,	10, 	1000);

    PHelper().CreateBOOL	(props,"Tools\\Box Pick\\Limited Depth",		&bp_lim_depth);
    PHelper().CreateBOOL	(props,"Tools\\Box Pick\\Back Face Culling",	&bp_cull);
    PHelper().CreateFloat	(props,"Tools\\Box Pick\\Depth Tolerance",		&bp_depth_tolerance,0.f, 	10000.f);
    PHelper().CreateFloat	(props,"Tools\\Sens\\Move",			          	&tools_sens_move);
    PHelper().CreateFloat	(props,"Tools\\Sens\\Rotate",		          	&tools_sens_rot);
    PHelper().CreateFloat	(props,"Tools\\Sens\\Scale",		          	&tools_sens_scale);
    PHelper().CreateAngle	(props,"Tools\\Snap\\Angle",		          	&snap_angle,		0, 		PI_MUL_2);
    PHelper().CreateFloat	(props,"Tools\\Snap\\Move",			          	&snap_move, 		0.01f,	1000.f);
    PHelper().CreateFloat	(props,"Tools\\Snap\\Move To", 		          	&snap_moveto,		0.01f,	1000.f);

    PHelper().CreateFlag32	(props,"Sounds\\Hardware",					    &psSoundFlags, 	ss_Hardware);
    PHelper().CreateFlag32	(props,"Sounds\\EAX",						    &psSoundFlags, 	ss_EAX);        

    PHelper().CreateFloat	(props,"Viewport\\Camera\\Move Sens",		    &cam_sens_move);
    PHelper().CreateFloat	(props,"Viewport\\Camera\\Rotate Sens",		    &cam_sens_rot);
    PHelper().CreateFloat	(props,"Viewport\\Camera\\Fly Speed",		    &cam_fly_speed, 	0.01f, 	100.f);
    PHelper().CreateFloat	(props,"Viewport\\Camera\\Fly Altitude",	    &cam_fly_alt, 		0.f, 	1000.f);
    PHelper().CreateColor	(props,"Viewport\\Fog\\Color",				    &fog_color	);
    PHelper().CreateFloat	(props,"Viewport\\Fog\\Fogness",			    &fog_fogness, 		0.f, 	100.f);
    PHelper().CreateFloat	(props,"Viewport\\Near Plane",				    &view_np, 			0.01f,	10.f);
    PHelper().CreateFloat	(props,"Viewport\\Far Plane", 				    &view_fp,			10.f, 	10000.f);
    PHelper().CreateAngle	(props,"Viewport\\FOV",		  				    &view_fov,			deg2rad(0.1f), deg2rad(170.f));
    PHelper().CreateColor	(props,"Viewport\\Clear Color",		           	&scene_clear_color	);

    PHelper().CreateFloatWithApply(props,"UI\\Interface Scale",             &ui_scale,          0.5f, 3.0f, 0.01f, 2,
        fastdelegate::MakeDelegate(this, &CCustomPreferences::OnUIScaleApply));

	{
		PropValue* V = PHelper().CreateBOOL(props, "UI\\Toolbar\\Vertical orientation", &toolbar_vertical);
		V->OnChangeEvent.bind(this, &CCustomPreferences::OnToolbarVerticalChange);
	}
	{
		PropValue* V = PHelper().CreateFloat(props, "UI\\Toolbar\\Icon scale", &toolbar_icon_scale, 0.5f, 3.0f, 0.1f, 1);
		V->OnChangeEvent.bind(this, &CCustomPreferences::OnToolbarIconScaleChange);
	}

    ButtonValue* B = PHelper().CreateButton	(props,"Keyboard\\Common\\File","Load,Save", 0);
    B->OnBtnClickEvent.bind	(this,&CCustomPreferences::OnKeyboardCommonFileClick);

    ECommandVec& cmds		= GetEditorCommands();
    for (u32 cmd_idx=0; cmd_idx<cmds.size(); cmd_idx++){
    	SECommand*& CMD		= cmds[cmd_idx];
        if (CMD&&CMD->editable){
        	VERIFY(!CMD->sub_commands.empty());
		    for (u32 sub_cmd_idx=0; sub_cmd_idx<CMD->sub_commands.size(); sub_cmd_idx++){
            	SESubCommand*& SUB_CMD = CMD->sub_commands[sub_cmd_idx];
                string128 nm; 		sprintf(nm,"%s%s%s",CMD->Desc(),!SUB_CMD->desc.empty()?"\\":"",SUB_CMD->desc.c_str());
                ShortcutValue* V 	= PHelper().CreateShortcut(props,PrepareKey("Keyboard\\Shortcuts",nm), &SUB_CMD->shortcut);
                V->OnValidateResultEvent.bind(CheckValidate);
            }
        }
    }
}

void CCustomPreferences::Edit()
{
    // fill prop
    PropItemVec props;
    FillProp						(props);
    
    IMProps->AssignItems(props);
    IMProps->Open();
    UI->AddIMWindow(IMProps);

    // save changed options
    Save							();
}

void CCustomPreferences::Load(CInifile* I)
{
    psDeviceFlags.flags		= R_U32_SAFE	("editor_preferences","device_flags",	psDeviceFlags.flags);
    psSoundFlags.flags		= R_U32_SAFE	("editor_preferences","sound_flags",	psSoundFlags.flags)

    Tools->m_Settings.flags	= R_U32_SAFE	("editor_preferences","tools_settings"  ,Tools->m_Settings.flags);
    
    view_np				= R_FLOAT_SAFE	("editor_preferences","view_np"			    ,view_np		 	);
    view_fp				= R_FLOAT_SAFE	("editor_preferences","view_fp"			    ,view_fp		 	);
    view_fov			= R_FLOAT_SAFE	("editor_preferences","view_fov"			,view_fov			);

    fog_color			= R_U32_SAFE	("editor_preferences","fog_color"			,fog_color			);
    fog_fogness			= R_FLOAT_SAFE	("editor_preferences","fog_fogness"		    ,fog_fogness	 	);

    cam_fly_speed		= R_FLOAT_SAFE	("editor_preferences","cam_fly_speed"		,cam_fly_speed		);
    cam_fly_alt			= R_FLOAT_SAFE	("editor_preferences","cam_fly_alt"		    ,cam_fly_alt	 	);
    cam_sens_rot		= R_FLOAT_SAFE	("editor_preferences","cam_sens_rot"		,cam_sens_rot		);
    cam_sens_move		= R_FLOAT_SAFE	("editor_preferences","cam_sens_move"		,cam_sens_move		);
	cam_undo_enabled    = R_BOOL_SAFE   ("editor_preferences","cam_undo_enabled"    ,TRUE               );
	cam_undo_duration   = R_FLOAT_SAFE  ("editor_preferences","cam_undo_duration"   ,0.3f               );

    tools_sens_move		= R_FLOAT_SAFE	("editor_preferences","tools_sens_move"	    ,tools_sens_move  	);
    tools_sens_rot		= R_FLOAT_SAFE	("editor_preferences","tools_sens_rot"	    ,tools_sens_rot		);
    tools_sens_scale	= R_FLOAT_SAFE	("editor_preferences","tools_sens_scale"	,tools_sens_scale	);
    
    bp_lim_depth		= R_BOOL_SAFE	("editor_preferences","bp_lim_depth"		,bp_lim_depth		);
    bp_cull				= R_BOOL_SAFE	("editor_preferences","bp_lim_depth"		,bp_cull		  	);  //.???
    bp_depth_tolerance	= R_FLOAT_SAFE	("editor_preferences","tools_sens_rot"	    ,bp_depth_tolerance	);  //.???

    snap_angle			= R_FLOAT_SAFE	("editor_preferences","snap_angle"		    ,snap_angle			);
    snap_move			= R_FLOAT_SAFE	("editor_preferences","snap_move"			,snap_move			);
    snap_moveto			= R_FLOAT_SAFE	("editor_preferences","snap_moveto"		    ,snap_moveto	   	);

    grid_cell_size		= R_FLOAT_SAFE	("editor_preferences","grid_cell_size"	    ,grid_cell_size		);
    grid_cell_count		= R_U32_SAFE	("editor_preferences","grid_cell_count"	    ,grid_cell_count   	);

    scene_undo_level	= R_U32_SAFE	("editor_preferences","scene_undo_level"	,scene_undo_level	);
    scene_recent_count	= R_U32_SAFE	("editor_preferences","scene_recent_count"  ,scene_recent_count	);
    scene_clear_color	= R_U32_SAFE	("editor_preferences","scene_clear_color"	,scene_clear_color	);

    object_flags.flags	= R_U32_SAFE	("editor_preferences","object_flags"		,object_flags.flags );

    ui_scale			= R_FLOAT_SAFE  ("editor_preferences", "ui_scale"           , 1.0f);

    toolbar_vertical	= R_BOOL_SAFE   ("editor_preferences", "toolbar_vertical"   , true);
	toolbar_icon_scale	= R_FLOAT_SAFE  ("editor_preferences", "toolbar_icon_scale" , 1.3f);

	IM_Storage& ws						   = GetSectionStorage("editor_main_window");
	windowState.wp.length				   = sizeof(WINDOWPLACEMENT);
	windowState.wp.showCmd				   = (UINT)ws.GetFloat("wp_showCmd", SW_SHOWNORMAL);
	windowState.wp.flags				   = (UINT)ws.GetFloat("wp_flags", 0);
	windowState.wp.ptMinPosition.x		   = -1;
	windowState.wp.ptMinPosition.y		   = -1;
	windowState.wp.ptMaxPosition.x		   = -1;
	windowState.wp.ptMaxPosition.y		   = -1;
	windowState.wp.rcNormalPosition.left   = (LONG)ws.GetFloat("wp_left", 100);
	windowState.wp.rcNormalPosition.top	   = (LONG)ws.GetFloat("wp_top", 100);
	windowState.wp.rcNormalPosition.right  = (LONG)ws.GetFloat("wp_right", 1280);
	windowState.wp.rcNormalPosition.bottom = (LONG)ws.GetFloat("wp_bottom", 720);

	windowState.ValidateAndFix();

    // read recent list    
    for (u32 i=0; i<scene_recent_count; i++){
        AnsiString key = "recent_files_" + std::to_string(i);
        shared_str fn  	= R_STRING_SAFE	("editor_preferences",key.c_str(),shared_str("") );
        if (fn.size())
        {
        	AStringIt it =   std::find(scene_recent_list.begin(), scene_recent_list.end(), fn.c_str() ) ;
            if (it==scene_recent_list.end())
	        	scene_recent_list.push_back(*fn);
        }
    }
    sWeather = R_STRING_SAFE	("editor_preferences", "weather", shared_str("") );
    // load shortcuts
    LoadShortcuts		(I);

    UI->LoadSettings	(I);
}

void CCustomPreferences::Save(CInifile* I)
{
    I->w_u32	("editor_preferences","device_flags",		psDeviceFlags.flags	);
    I->w_u32	("editor_preferences","sound_flags",		psSoundFlags.flags	);

    I->w_u32	("editor_preferences","tools_settings",	    Tools->m_Settings.flags	);

    I->w_float	("editor_preferences","view_np",			view_np			);
    I->w_float	("editor_preferences","view_fp",			view_fp			);
    I->w_float	("editor_preferences","view_fov",			view_fov		);

    I->w_u32	("editor_preferences","fog_color",		    fog_color		);
    I->w_float	("editor_preferences","fog_fogness",		fog_fogness		);

    I->w_float	("editor_preferences","cam_fly_speed",	    cam_fly_speed	);
    I->w_float	("editor_preferences","cam_fly_alt",		cam_fly_alt		);
    I->w_float	("editor_preferences","cam_sens_rot",		cam_sens_rot	);
    I->w_float	("editor_preferences","cam_sens_move",	    cam_sens_move	);
	I->w_bool   ("editor_preferences","cam_undo_enabled",   cam_undo_enabled);
	I->w_float  ("editor_preferences","cam_undo_duration",  cam_undo_duration);

    I->w_float	("editor_preferences","tools_sens_rot",	    tools_sens_rot	);
    I->w_float	("editor_preferences","tools_sens_move",	tools_sens_move	);
    I->w_float	("editor_preferences","tools_sens_scale",	tools_sens_scale);

    I->w_bool	("editor_preferences","bp_lim_depth",		bp_lim_depth	);
    I->w_bool	("editor_preferences","bp_lim_depth",		bp_cull			);
    I->w_float	("editor_preferences","bp_depth_tolerance", bp_depth_tolerance	);

    I->w_float	("editor_preferences","snap_angle",		    snap_angle		);
    I->w_float	("editor_preferences","snap_move",		    snap_move		);
    I->w_float	("editor_preferences","snap_moveto",		snap_moveto		);

    I->w_float	("editor_preferences","grid_cell_size",	    grid_cell_size	);
    I->w_u32	("editor_preferences","grid_cell_count",	grid_cell_count	);

    I->w_u32	("editor_preferences","scene_undo_level",	scene_undo_level	);
    I->w_u32	("editor_preferences","scene_recent_count",	scene_recent_count	);
    I->w_u32	("editor_preferences","scene_clear_color",	scene_clear_color 	);

    I->w_u32	("editor_preferences","object_flags",		object_flags.flags);

    I->w_float  ("editor_preferences", "ui_scale",          ui_scale);

    I->w_bool   ("editor_preferences", "toolbar_vertical",   toolbar_vertical);
	I->w_float  ("editor_preferences", "toolbar_icon_scale", toolbar_icon_scale);

    for (AStringIt it=scene_recent_list.begin(); it!=scene_recent_list.end(); it++){
        AnsiString L = "recent_files_" + std::to_string(it - scene_recent_list.begin());
        AnsiString V = '"' + *it + '"';
        I->w_string("editor_preferences",L.c_str(),V.c_str());
    }
    I->w_string("editor_preferences","weather",   sWeather.c_str() );
    // load shortcuts
    SaveShortcuts		(I);
    UI->SaveSettings	(I);
}

void CCustomPreferences::Load()
{
	IM_Storage& main = GetSectionStorage("editor_preferences");
	CInifile*	I	 = main.GetIni();

	Load(I);
	ApplyValues();
}

void CCustomPreferences::Save()
{
	IM_Storage& main = GetSectionStorage("editor_preferences");
	CInifile*	I	 = main.GetIni();

	Save(I);
	main.Save();
}

IM_Storage& CCustomPreferences::GetSectionStorage(LPCSTR section) {
		string_path path;
		INI_NAME(path);

		auto it = m_SectionStorages.find(section);
		if (it == m_SectionStorages.end()) {
			IM_Storage* storage		   = xr_new<IM_Storage>(true, xr_string(path), xr_string(section));
			m_SectionStorages[section] = storage;
			return *storage;
		}
		return *it->second;
	}

void CCustomPreferences::AppendRecentFile(LPCSTR name)
{
    for (AStringIt it=scene_recent_list.begin(); it!=scene_recent_list.end(); it++){
    	if (*it==name){
        	scene_recent_list.erase	(it);
            break;
        }
    }
    scene_recent_list.insert(scene_recent_list.begin(),name);
    while (scene_recent_list.size()>=EPrefs->scene_recent_count) 
        scene_recent_list.pop_back();

    ExecCommand				(COMMAND_REFRESH_UI_BAR);
}
//---------------------------------------------------------------------------

void CCustomPreferences::OnCreate()
{
    Load				();
    
    IMProps = xr_new<IM_PropertiesWnd>("Editor Preferences");
    IMProps->Modal = true;
    IMProps->ShowButtonsBar = false;
    IMProps->OnClose.bind(this, &CCustomPreferences::OnClose);
}
//---------------------------------------------------------------------------

void CCustomPreferences::OnDestroy()
{
    xr_delete		(IMProps);
    Save				();
}
//---------------------------------------------------------------------------
