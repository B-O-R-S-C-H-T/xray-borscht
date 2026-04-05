#include "stdafx.h"
#pragma hdrstop

#include "IM_Manipulator.h"

#include <imgui.h>
#include <addons/ImGuizmo/ImGuizmo.h>

#include "../Edit/CustomObject.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/scene.h"
#include "../Edit/WayPoint.h"
#include "../Edit/ESceneWayControls.h"

IM_Manipulator imManipulator;

void IM_Manipulator::Render() {
	//ELog.Msg(mtInformation, "%d", LTools->CurrentClassID());
	ESceneCustomOTool* tool = Scene->GetOTool(LTools->CurrentClassID());
	if (!tool)
		return;

	ObjectList lst;
	tool->GetQueryObjects(lst, TRUE, TRUE, FALSE);
	if (lst.size() < 1)
		return;

	if (LTools->CurrentClassID() == OBJCLASS_WAY) {
		if (LTools->GetAction() != etaMove) {
			return;
		}
	}

	bool bWorld = Tools->GetSettings(etfCSParent);

	ImGuizmo::OPERATION op;
	ImGuizmo::MODE	mode;
	float*			snap = NULL;

	ImGuizmo::SetRect(0.0f, 0.0f, static_cast<float>(Device.m_RealWidth), static_cast<float>(Device.m_RealHeight));

	switch (LTools->GetAction()) {
		case etaMove:
		{
			op	 = ImGuizmo::TRANSLATE;
			mode = bWorld ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
			snap = LTools->GetSettings(etfMSnap) ? &Tools->m_MoveSnap : NULL;

			CCustomObject* first = lst.front();
			Fmatrix		   mat   = first->FTransformRP;

			CWayObject* way = dynamic_cast<CWayObject*>(first);
			if (way) {
				Fvector pos;
				if ((LTools->CurrentClassID() == OBJCLASS_WAY) && (LTools->GetSubTarget() == estWayModePoint)) {
					CWayPoint* wp = way->GetFirstSelected();
					pos = (wp != nullptr) ? wp->GetPosition() : way->GetPosition();
				} else {
					pos = way->GetPosition();
				}
				mat = Fidentity;
				mat.translate(pos);
			}

			Fmatrix delta = Fidentity;
			bool m	= ImGuizmo::Manipulate((float*)&Device.mView, (float*)&Device.mProjection, op, mode, (float*)&mat, (float*)&delta, snap);

			if (m) {
				if (!m_moving) {
					m_oldPositionValues.clear();
					for (ObjectIt it = lst.begin(); it != lst.end(); ++it) {
						m_oldPositionValues.push_back((*it)->GetPosition());
					}

					m_oldCamPos = Device.m_Camera.GetPosition();
					m_oldCamHPB = Device.m_Camera.GetHPB();

					m_moving	= true;
				}

				if (bWorld) {
					for (ObjectIt it = lst.begin(); it != lst.end(); it++)
						(*it)->Move(delta.c);
				} else {
					Fmatrix rot_inv		 = Fmatrix().invert(lst.front()->FTransformR);
					Fmatrix local_offset = Fmatrix().mul(rot_inv, delta);

					for (ObjectIt it = lst.begin(); it != lst.end(); it++) {
						Fmatrix offset{};
						offset.mul((*it)->FTransformR, local_offset);
						(*it)->Move(offset.c);
					}
				}
			}
		} break;

		case etaRotate:
		{
			op			= ImGuizmo::ROTATE;
			mode		= bWorld ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

			float ang	= rad2deg(Tools->m_RotateSnapAngle);
			snap		= LTools->GetSettings(etfASnap) ? &ang : NULL;

			//if(lst.size() > 1) {
			//	return;
			//}
			Fmatrix mat = lst.front()->FTransformRP, diff = Fidentity;

			Fvector original_rot;
			mat.getXYZ(original_rot);

			bool m = ImGuizmo::Manipulate((float*)&Device.mView, (float*)&Device.mProjection, op, mode, (float*)&mat, (float*)&diff, snap);

			if (m) {
				if (!m_rotating) {
					m_oldRotateValues.clear();
					for (ObjectIt it = lst.begin(); it != lst.end(); ++it) {
						m_oldRotateValues.push_back((*it)->GetRotation());
					}

					m_oldCamPos = Device.m_Camera.GetPosition();
					m_oldCamHPB = Device.m_Camera.GetHPB();

					m_rotating  = true;
				}

				if (bWorld) {
					for (ObjectIt it = lst.begin(); it != lst.end(); it++) {
						Fmatrix t{};
						t.mul(diff, (*it)->FTransformR);

						Fvector rot;
						t.getXYZ(rot);

						(*it)->SetRotation(rot);
					}
				} else {
					Fvector rot;
					mat.getXYZ(rot);

					Fvector rot_diff{};
					rot_diff.set(rot).sub(original_rot);

					//ELog.Msg(mtInformation, "%f %f %f", rot_diff.x, rot_diff.y, rot_diff.z);

					for (ObjectIt it = lst.begin(); it != lst.end(); it++) {
						(*it)->RotateParent(Fvector().set(0, 0, 1), rot_diff.z);
						(*it)->RotateParent(Fvector().set(1, 0, 0), rot_diff.x);
						(*it)->RotateParent(Fvector().set(0, 1, 0), rot_diff.y);
					}
				}
			}
		} break;

		case etaScale:
		{
			op	 = ImGuizmo::SCALE;
			mode = ImGuizmo::LOCAL;

			//if(lst.size() > 1) {
			//	return;
			//}

			Fmatrix mat = lst.front()->FTransform;

			Fvector original_scale{};
			original_scale.x = mat.i.magnitude();
			original_scale.y = mat.j.magnitude();
			original_scale.z = mat.k.magnitude();

			bool m = ImGuizmo::Manipulate((float*)&Device.mView, (float*)&Device.mProjection, op, mode, (float*)&mat);

			if (m) {
				if (!m_scaling) {
					m_oldScaleValues.clear();
					for (ObjectIt it = lst.begin(); it != lst.end(); ++it) {
						m_oldScaleValues.push_back((*it)->GetScale());
					}

					m_oldCamPos = Device.m_Camera.GetPosition();
					m_oldCamHPB = Device.m_Camera.GetHPB();

					m_scaling	= true;
				}

				Fvector scale{};
				scale.x = mat.i.magnitude() - original_scale.x;
				scale.y = mat.j.magnitude() - original_scale.y;
				scale.z = mat.k.magnitude() - original_scale.z;

				for (ObjectIt it = lst.begin(); it != lst.end(); it++)
					(*it)->Scale(scale);
			}
		} break;
	}

	if (ImGuizmo::IsUsing() && !m_active) {
		m_active = true;
	}

	if (!ImGuizmo::IsUsing() && m_active) {
		ObjectList lst;
		tool->GetQueryObjects(lst, TRUE, TRUE, FALSE);

		if (m_moving && !m_oldPositionValues.empty()) {
			std::vector<CCustomObject*> objects;
			std::vector<Fvector>		newPositions;

			for (ObjectIt it = lst.begin(); it != lst.end(); ++it) {
				objects.push_back(*it);
				newPositions.push_back((*it)->GetPosition());
			}

			if (!objects.empty()) {
				auto cmd = std::make_unique<MoveCommand>(objects, m_oldPositionValues, newPositions, m_oldCamPos, m_oldCamHPB);
				Scene->PushCommand(std::move(cmd));
			}

			m_oldPositionValues.clear();
			m_moving = false;
		}

		if (m_rotating && !m_oldRotateValues.empty()) {
			std::vector<CCustomObject*> objects;
			std::vector<Fvector>		newRotations;

			for (ObjectIt it = lst.begin(); it != lst.end(); ++it) {
				objects.push_back(*it);
				newRotations.push_back((*it)->GetRotation());
			}

			if (!objects.empty()) {
				auto cmd = std::make_unique<RotateCommand>(objects, m_oldRotateValues, newRotations, m_oldCamPos, m_oldCamHPB);
				Scene->PushCommand(std::move(cmd));
			}

			m_oldRotateValues.clear();
			m_rotating = false;
		}

		if (m_scaling && !m_oldScaleValues.empty()) {
			std::vector<CCustomObject*> objects;
			std::vector<Fvector>		newScales;

			for (ObjectIt it = lst.begin(); it != lst.end(); ++it) {
				objects.push_back(*it);
				newScales.push_back((*it)->GetScale());
			}

			if (!objects.empty()) {
				auto cmd = std::make_unique<ScaleCommand>(objects, m_oldScaleValues, newScales, m_oldCamPos, m_oldCamHPB);
				Scene->PushCommand(std::move(cmd));
			}

			m_oldScaleValues.clear();
			m_scaling = false;
		}

		m_active = false;
	}
}
