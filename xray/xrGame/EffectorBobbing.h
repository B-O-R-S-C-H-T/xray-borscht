#ifndef _EFFECTOR_BOBBING_H
#define _EFFECTOR_BOBBING_H
#pragma once

#include "CameraEffector.h"
#include "../xrEngine/cameramanager.h"

class CEffectorBobbing : public CEffectorCam  
{
	float	fTime;
//	Fvector	vAngleAmplitude;	// рудимент ТСС?
//	float	fYAmplitude;		// рудимент ТСС?
//	float	fSpeed;				// рудимент ТСС?

	u32		dwMState;
	float	fReminderFactor;
	bool	is_limping;
	bool	m_bZoomMode;

	float	m_fAmplitudeRun;
	float	m_fAmplitudeWalk;
	float	m_fAmplitudeLimp;

	float	m_fSpeedRun;
	float	m_fSpeedWalk;
	float	m_fSpeedLimp;

public:
			CEffectorBobbing	();
	virtual ~CEffectorBobbing	();
	virtual BOOL	ProcessCam	(SCamEffectorInfo& info);
	void	SetState			(u32 st, bool limping, bool ZoomMode);

private:
	static constexpr const char* kSectionName	= "bobbing_effector";
	static constexpr float		 kCrouchFactor	= 0.75f;
	static constexpr float		 kSpeedReminder = 5.f;
};

#endif //_EFFECTOR_BOBBING_H
