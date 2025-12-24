#ifndef __CAMERA_FE_H__
#define __CAMERA_FE_H__

#include "../xrEngine/CameraBase.h"

class CCameraFirstEye : public CCameraBase {
	typedef CCameraBase inherited;
	Fvector			lookat_point;
	bool			lookat_active;
	void			UpdateLookat	();

public:
					CCameraFirstEye	( CObject* p, u32 flags=0);
	virtual			~CCameraFirstEye( );

	virtual void	Load			(LPCSTR section);
	virtual void	Move			( int cmd, float val=0, float factor=1.0f );

	virtual	void	OnActivate		( CCameraBase* old_cam );
	virtual void	Update			( Fvector& point, Fvector& noise_dangle );

	virtual float	GetWorldYaw		( )	{ return -yaw;	};
	virtual float	GetWorldPitch	( )	{ return pitch; };
			void	LookAtPoint		(Fvector p){lookat_point=p;lookat_active=true;}

private:
			bool	m_bInterpolating;
			float	m_fInterpElapsed;
			float	m_fInterpDuration;
			Fvector	m_vInterpStartPos;
			Fvector	m_vInterpStartDir;
			Fvector	m_vInterpStartNorm;
			bool	m_bStartFromThird;
			Fvector m_vThirdPos;
			Fvector m_vThirdDir;
			Fvector m_vThirdNorm;
			bool	m_bUseBoneBased; // использовать костно-ориентированную или обычную

public:
			void	GetBoneBasedPosition(Fvector& pos, Fvector& dir, Fvector& norm) const override;
			void	StartInterpolation(float duration) override;
			void	SetStartFromLookAt	(Fvector pos, Fvector dir, Fvector norm);
			void	SetUseBoneBased		(bool use) { m_bUseBoneBased = use; }
			bool	GetUseBoneBased		() const { return m_bUseBoneBased; }
};

#endif // __CAMERA_FE_H__