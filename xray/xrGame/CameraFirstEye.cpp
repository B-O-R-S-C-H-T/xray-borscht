#include "stdafx.h"

#include "CameraFirstEye.h"

#include "xr_level_controller.h"

#include "../Include/xrRender/Kinematics.h"
#include "../xrEngine/xr_object.h"
#include "actor.h"

CCameraFirstEye::CCameraFirstEye(CObject* p, u32 flags) : CCameraBase(p, flags), lookat_active(false) {}

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
		IKinematics* V = smart_cast<IKinematics*>(actor->Visual());
		if (!V)
			return;
		V->CalculateBones();
		u16		head	  = actor->GetHeadBoneID();
		Fmatrix mHead	  = V->LL_GetTransform(head);
		Fvector eye_local = actor->m_fEyeOffset;
		Fvector eye_world;
		mHead.transform_tiny(eye_world, eye_local);
		Fmatrix actor_xform = actor->XFORM();
		actor_xform.transform_tiny(eye_world);
		vPosition.set(eye_world);

		Fvector dir_local{};
		dir_local.set(0, 0, 1);
		Fvector dir_world;
		mHead.transform_dir(dir_world, dir_local);
		actor_xform.transform_dir(dir_world);
		vDirection.set(dir_world);

		Fvector up_local{};
		up_local.set(1, 0, 0);
		Fvector up_world;
		mHead.transform_dir(up_world, up_local);
		actor_xform.transform_dir(up_world);
		vNormal.set(up_world);

	} else { // нормальный режим
		vPosition.set		(point);

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

		vDirection.set		(mR.k);
		vNormal.set			(mR.j);
	}

	if (m_Flags.is(flRelativeLink)) {
		parent->XFORM().transform_dir(vDirection);
		parent->XFORM().transform_dir(vNormal);
	}
}

void CCameraFirstEye::Move( int cmd, float val, float factor )
{
	if (bClampPitch)
	{
		while (pitch < lim_pitch[0])
			pitch += PI_MUL_2;
		while (pitch > lim_pitch[1])
			pitch -= PI_MUL_2;
	};
	switch (cmd){
	case kDOWN:		pitch	-= val?val:(rot_speed.y*Device.fTimeDelta/factor);	break;
	case kUP:		pitch	+= val?val:(rot_speed.y*Device.fTimeDelta/factor);	break;
	case kLEFT:		yaw		-= val?val:(rot_speed.x*Device.fTimeDelta/factor);	break;
	case kRIGHT:	yaw		+= val?val:(rot_speed.x*Device.fTimeDelta/factor);	break;
	}
	if (bClampYaw)		clamp(yaw,lim_yaw[0],lim_yaw[1]);
	if (bClampPitch)	clamp(pitch,lim_pitch[0],lim_pitch[1]);
}

void CCameraFirstEye::OnActivate( CCameraBase* old_cam )
{
	if (old_cam&&(m_Flags.is(flRelativeLink)==old_cam->m_Flags.is(flRelativeLink)))
		yaw = (old_cam)->yaw;
	if (yaw > PI_MUL_2) yaw -= PI_MUL_2;
	if (yaw < -PI_MUL_2)yaw += PI_MUL_2;
}