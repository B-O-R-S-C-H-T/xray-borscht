#include "stdafx.h"

#include "..\ECore\Editor\undo_manager.h"
#include "scene.h"

void UndoManager::Push(std::unique_ptr<ICommand> cmd) {
	m_undoStack.push_back(std::move(cmd));
	m_redoStack.clear();
}

void UndoManager::Undo() {
	if (m_undoStack.empty())
		return;
	auto cmd = std::move(m_undoStack.back());
	m_undoStack.pop_back();
	cmd->Undo();
	m_redoStack.push_back(std::move(cmd));
}

void UndoManager::Redo() {
	if (m_redoStack.empty())
		return;
	auto cmd = std::move(m_redoStack.back());
	m_redoStack.pop_back();
	cmd->Redo();
	m_undoStack.push_back(std::move(cmd));
}

void UndoManager::Clear() {
	m_undoStack.clear();
	m_redoStack.clear();
}

MoveCommand::MoveCommand(const std::vector<CCustomObject*>& objects,
						 const std::vector<Fvector>&		oldPositions,
						 const std::vector<Fvector>&		newPositions,
						 const Fvector&						oldCamPos,
						 const Fvector&						oldCamHPB)
	: m_objects(objects), m_oldPositions(oldPositions), m_newPositions(newPositions), m_oldCamPos(oldCamPos), m_oldCamHPB(oldCamHPB),
	  m_newCamPos(Device.m_Camera.GetPosition()), m_newCamHPB(Device.m_Camera.GetHPB()) {
	VERIFY(m_objects.size() == m_oldPositions.size());
	VERIFY(m_objects.size() == m_newPositions.size());
}

void MoveCommand::Undo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetPosition(m_oldPositions[i]);
	}

	if (EPrefs->cam_undo_enabled) {
		Device.m_Camera.StartUndoCamInterpolation(m_oldCamPos, m_oldCamHPB, EPrefs->cam_undo_duration);
	} else {
		Device.m_Camera.Set(m_oldCamHPB, m_oldCamPos);
		Device.m_Camera.BuildCamera();
	}
}

void MoveCommand::Redo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetPosition(m_newPositions[i]);
	}

	if (EPrefs->cam_undo_enabled) {
		Device.m_Camera.StartUndoCamInterpolation(m_newCamPos, m_newCamHPB, EPrefs->cam_undo_duration);
	} else {
		Device.m_Camera.Set(m_newCamHPB, m_newCamPos);
		Device.m_Camera.BuildCamera();
	}
}

ScaleCommand::ScaleCommand(const std::vector<CCustomObject*>& objects,
						   const std::vector<Fvector>&		  oldScales,
						   const std::vector<Fvector>&		  newScales,
						   const Fvector&					  oldCamPos,
						   const Fvector&					  oldCamHPB)
	: m_objects(objects), m_oldScales(oldScales), m_newScales(newScales), m_oldCamPos(oldCamPos), m_oldCamHPB(oldCamHPB),
	  m_newCamPos(Device.m_Camera.GetPosition()), m_newCamHPB(Device.m_Camera.GetHPB()) {
	VERIFY(m_objects.size() == m_oldScales.size());
	VERIFY(m_objects.size() == m_newScales.size());
}

void ScaleCommand::Undo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetScale(m_oldScales[i]);
	}

	if (EPrefs->cam_undo_enabled) {
		Device.m_Camera.StartUndoCamInterpolation(m_oldCamPos, m_oldCamHPB, EPrefs->cam_undo_duration);
	} else {
		Device.m_Camera.Set(m_oldCamHPB, m_oldCamPos);
		Device.m_Camera.BuildCamera();
	}
}

void ScaleCommand::Redo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetScale(m_newScales[i]);
	}

	if (EPrefs->cam_undo_enabled) {
		Device.m_Camera.StartUndoCamInterpolation(m_newCamPos, m_newCamHPB, EPrefs->cam_undo_duration);
	} else {
		Device.m_Camera.Set(m_newCamHPB, m_newCamPos);
		Device.m_Camera.BuildCamera();
	}
}

RotateCommand::RotateCommand(const std::vector<CCustomObject*>& objects,
							 const std::vector<Fvector>&		oldRotations,
							 const std::vector<Fvector>&		newRotations,
							 const Fvector&						oldCamPos,
							 const Fvector&						oldCamHPB)
	: m_objects(objects), m_oldRotations(oldRotations), m_newRotations(newRotations), m_oldCamPos(oldCamPos), m_oldCamHPB(oldCamHPB),
	  m_newCamPos(Device.m_Camera.GetPosition()), m_newCamHPB(Device.m_Camera.GetHPB()) {
	VERIFY(m_objects.size() == m_oldRotations.size());
	VERIFY(m_objects.size() == m_newRotations.size());
}

void RotateCommand::Undo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetRotation(m_oldRotations[i]);
	}

	if (EPrefs->cam_undo_enabled) {
		Device.m_Camera.StartUndoCamInterpolation(m_oldCamPos, m_oldCamHPB, EPrefs->cam_undo_duration);
	} else {
		Device.m_Camera.Set(m_oldCamHPB, m_oldCamPos);
		Device.m_Camera.BuildCamera();
	}
}

void RotateCommand::Redo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetRotation(m_newRotations[i]);
	}

	if (EPrefs->cam_undo_enabled) {
		Device.m_Camera.StartUndoCamInterpolation(m_newCamPos, m_newCamHPB, EPrefs->cam_undo_duration);
	} else {
		Device.m_Camera.Set(m_newCamHPB, m_newCamPos);
		Device.m_Camera.BuildCamera();
	}
}

PasteCommand::PasteCommand(std::vector<CCustomObject*> added) : m_added(std::move(added)) {}

void PasteCommand::Undo() {
	for (auto obj : m_added) {
		if (obj) {
			if (obj->OnSelectionRemove())
				Scene->RemoveObject(obj, false, false);
		}
	}
	m_undone = true;
}

void PasteCommand::Redo() {
	for (auto obj : m_added) {
		if (obj) {
			Scene->AppendObject(obj, false);
			obj->Select(true);
		}
	}
	m_undone = false;
}

PasteCommand::~PasteCommand() {
	if (m_undone) {
		for (auto obj : m_added) {
			xr_delete(obj);
		}
	}
}

CutCommand::CutCommand(std::vector<CCustomObject*> removed) : m_removed(std::move(removed)) {}

void CutCommand::Redo() {
	for (auto obj : m_removed) {
		if (obj && obj->OnSelectionRemove())
			Scene->RemoveObject(obj, false, true);
	}
	m_cut = true;
}

void CutCommand::Undo() {
	for (auto obj : m_removed) {
		if (obj) {
			Scene->AppendObject(obj, false);
			obj->Select(true);
		}
	}
	m_cut = false;
}

CutCommand::~CutCommand() {
	if (m_cut) {
		for (auto obj : m_removed) {
			xr_delete(obj);
		}
	}
}
