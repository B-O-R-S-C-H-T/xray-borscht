#include "stdafx.h"

#include "CameraFirstEye.h"

#include "xr_level_controller.h"

#include "../Include/xrRender/Kinematics.h"
#include "../xrEngine/xr_object.h"
#include "actor.h"

CCameraFirstEye::CCameraFirstEye(CObject* p, u32 flags) : CCameraBase(p, flags) {
	lookat_active	  = false;
	m_bInterpolating  = false;
	m_fInterpElapsed  = 0.0f;
	m_fInterpDuration = 0.0f;
}

CCameraFirstEye::~CCameraFirstEye() {}

void CCameraFirstEye::Load(LPCSTR section) {
	inherited::Load		(section);
	style				= csFirstEye;
}

void CCameraFirstEye::UpdateLookat() {
	if(!lookat_active)
		return;

	Fvector								_dest_dir{};
	_dest_dir.sub						(lookat_point, vPosition);

	Fmatrix								_m{};
	_m.identity							();
	_m.k.normalize_safe					(_dest_dir);
	Fvector::generate_orthonormal_basis	(_m.k, _m.j, _m.i);

	Fvector								xyz;
	_m.getXYZi							(xyz);

	if(fsimilar(yaw, xyz.y, EPS) && fsimilar(pitch, xyz.x, EPS))
		lookat_active = false;

	yaw				= angle_inertion_var(	yaw, xyz.y,
											1.0f,
											PI,
											PI,
											Device.fTimeDelta);

	pitch			= angle_inertion_var(	pitch, xyz.x,
											1.0f,
											PI,
											PI,
											Device.fTimeDelta);
}

void CCameraFirstEye::Update(Fvector& point, Fvector& noise_dangle) {
	CActor* actor = smart_cast<CActor*>(parent);
	if (actor && (!actor->g_Alive() || actor->m_bFainted || actor->m_bRecoveringFromFaint)) { // режим смерти или обморока
		Fvector pos, dir, norm;
		GetBoneBasedPosition(pos, dir, norm);
		vPosition  = pos;
		vDirection = dir;
		vNormal	   = norm;
	} else { // нормальный режим
		vPosition.set(point);

		Fmatrix mR{}, R{};
		Fmatrix rX{}, rY{}, rZ{};
		rX.rotateX(noise_dangle.x);
		rY.rotateY(-noise_dangle.y);
		rZ.rotateZ(noise_dangle.z);
		R.mul_43(rY, rX);
		R.mulB_43(rZ);

		mR.identity();
		Fquaternion Q{};
		Q.rotationYawPitchRoll(roll, yaw, pitch);
		mR.rotation(Q);
		mR.transpose();
		mR.mulB_43(R);

		vDirection.set(mR.k);
		vNormal.set(mR.j);
	}

	// интерполяция
	if (m_bInterpolating) {
		Fvector target_pos	= vPosition;
		Fvector target_dir	= vDirection;
		Fvector target_norm = vNormal;

		float	t			= m_fInterpElapsed / m_fInterpDuration;
		clamp(t, 0.0f, 1.0f);

		vPosition.lerp(m_vInterpStartPos, target_pos, t);
		vDirection.lerp(m_vInterpStartDir, target_dir, t);
		vDirection.normalize_safe();
		vNormal.lerp(m_vInterpStartNorm, target_norm, t);
		vNormal.normalize_safe();

		m_fInterpElapsed += Device.fTimeDelta;
		if (m_fInterpElapsed >= m_fInterpDuration) {
			m_bInterpolating = false;
			Msg("[DEBUG] Camera interpolation finished in CCameraFirstEye.");
		}
	}

	if (m_Flags.is(flRelativeLink)) {
		parent->XFORM().transform_dir(vDirection);
		parent->XFORM().transform_dir(vNormal);
	}
}

void CCameraFirstEye::GetBoneBasedPosition(Fvector& pos, Fvector& dir, Fvector& norm) const {
	CActor* actor = smart_cast<CActor*>(parent);
	if (!actor || actor->GetHeadBoneID() == u16(-1)) {
		pos	 = vPosition;
		dir	 = vDirection;
		norm = vNormal;
		return;
	}

	IKinematics* K = smart_cast<IKinematics*>(actor->Visual());
	if (!K)
		return;

	K->CalculateBones(TRUE);

	Fmatrix mHead	 = K->LL_GetTransform(actor->GetHeadBoneID());
	Fvector head_pos = mHead.c;
	parent->XFORM().transform_tiny(head_pos);

	Fvector eye_pos = mHead.c;
	mHead.transform_tiny(eye_pos, actor->m_fEyeOffset);
	parent->XFORM().transform_tiny(eye_pos);

	dir.sub(eye_pos, head_pos);
	dir.normalize_safe();

	Fvector up_local = {1, 0, 0}; // именно такой порядок!
	mHead.transform_dir(norm, up_local);
	parent->XFORM().transform_dir(norm);
	norm.normalize_safe();

	pos = eye_pos;
}

void CCameraFirstEye::StartInterpolation(float duration) {
	m_fInterpDuration = duration;
	m_fInterpElapsed = 0.0f;

	GetBoneBasedPosition(m_vInterpStartPos, m_vInterpStartDir, m_vInterpStartNorm);

	m_bInterpolating = true;
}

void CCameraFirstEye::Move( int cmd, float val, float factor ) {
	if (bClampPitch) {
		while (pitch < lim_pitch[0])
			pitch += PI_MUL_2;
		while (pitch > lim_pitch[1])
			pitch -= PI_MUL_2;
	}
	switch (cmd){
	case kDOWN:		pitch	-= val?val:(rot_speed.y*Device.fTimeDelta/factor);	break;
	case kUP:		pitch	+= val?val:(rot_speed.y*Device.fTimeDelta/factor);	break;
	case kLEFT:		yaw		-= val?val:(rot_speed.x*Device.fTimeDelta/factor);	break;
	case kRIGHT:	yaw		+= val?val:(rot_speed.x*Device.fTimeDelta/factor);	break;
	}
	if (bClampYaw)		clamp(yaw,lim_yaw[0],lim_yaw[1]);
	if (bClampPitch)	clamp(pitch,lim_pitch[0],lim_pitch[1]);
}

void CCameraFirstEye::OnActivate( CCameraBase* old_cam ) {
	if (old_cam&&(m_Flags.is(flRelativeLink)==old_cam->m_Flags.is(flRelativeLink)))
		yaw = (old_cam)->yaw;
	if (yaw > PI_MUL_2) yaw -= PI_MUL_2;
	if (yaw < -PI_MUL_2)yaw += PI_MUL_2;
}