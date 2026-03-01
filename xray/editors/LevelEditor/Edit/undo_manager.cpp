///////////////////////////////////////////////////////////////////////////////
// undo_manager.cpp
// Командно-ориентированная система отмены/повтора действий
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <editors/ECore/Editor/undo_manager.h>

#include "scene.h"
#include "SceneObject.h"

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
	for (auto& cmd : m_undoStack)
		if (cmd)
			cmd->ReleaseResources();
	for (auto& cmd : m_redoStack)
		if (cmd)
			cmd->ReleaseResources();
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
		if (obj && Scene->ContainsObject(obj, obj->ClassID)) {
			Scene->RemoveObject(obj, false, false);
		}
	}
}

void PasteCommand::Redo() {
	for (auto obj : m_added) {
		if (obj && !obj->IsDeleted() && !Scene->ContainsObject(obj, obj->ClassID)) {
			Scene->AppendObject(obj, false);
			obj->Select(true);
		}
	}
}

PasteCommand::~PasteCommand() {
	m_added.clear();
}

DuplicateCommand::DuplicateCommand(std::vector<CCustomObject*> added) : m_added(std::move(added)) {}

void DuplicateCommand::Undo() {
	for (auto obj : m_added) {
		if (obj && Scene->ContainsObject(obj, obj->ClassID)) {
			Scene->RemoveObject(obj, false, false);
		}
	}
}

void DuplicateCommand::Redo() {
	for (auto obj : m_added) {
		if (obj && !obj->IsDeleted() && !Scene->ContainsObject(obj, obj->ClassID)) {
			Scene->AppendObject(obj, false);
			obj->Select(true);
		}
	}
}

DuplicateCommand::~DuplicateCommand() {
	m_added.clear();
}

CutCommand::CutCommand(std::vector<CCustomObject*> objects) : m_objects(std::move(objects)) {}

void CutCommand::Redo() {
	for (auto obj : m_objects) {
		if (obj)
			Scene->RemoveObject(obj, false, false);
	}
	m_cut = true;
}

void CutCommand::Undo() {
	for (auto obj : m_objects) {
		if (obj) {
			Scene->AppendObject(obj, false);
			obj->Select(true);
		}
	}
	m_cut = false;
}

CutCommand::~CutCommand() {
	if (m_cut) {
		for (auto obj : m_objects) {
			xr_delete(obj);
		}
	}
	m_objects.clear();
}

AddObjectCommand::AddObjectCommand(CCustomObject* obj) : m_object(obj) {}

void AddObjectCommand::Undo() {
	if (m_object)
		Scene->RemoveObject(m_object, false, false);
}

void AddObjectCommand::Redo() {
	if (m_object) {
		Scene->AppendObject(m_object, false);
		m_object->Select(true);
	}
}

AddObjectCommand::~AddObjectCommand() {
	m_object = nullptr;
}

RemoveObjectsCommand::RemoveObjectsCommand(std::vector<CCustomObject*> objects) : m_objects(std::move(objects)) {}

void RemoveObjectsCommand::Undo() {
	for (auto obj : m_objects) {
		if (obj) {
			Scene->AppendObject(obj, false);
			obj->Select(true);
		}
	}
}

void RemoveObjectsCommand::Redo() {
	for (auto obj : m_objects) {
		if (obj)
			Scene->RemoveObject(obj, false, false);
	}
}

void RemoveObjectsCommand::ReleaseResources() {
	for (auto obj : m_objects) {
		if (obj)
			xr_delete(obj);
	}
	m_objects.clear();
}

RenameCommand::RenameCommand(std::vector<CCustomObject*> objects, std::vector<xr_string> oldNames, std::vector<xr_string> newNames)
	: m_objects(std::move(objects)), m_oldNames(std::move(oldNames)), m_newNames(std::move(newNames)) {
	VERIFY(m_objects.size() == m_oldNames.size());
	VERIFY(m_objects.size() == m_newNames.size());
}

void RenameCommand::Undo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetName(m_oldNames[i].c_str());
	}
}

void RenameCommand::Redo() {
	for (size_t i = 0; i < m_objects.size(); ++i) {
		if (m_objects[i])
			m_objects[i]->SetName(m_newNames[i].c_str());
	}
}

HideCommand::HideCommand(std::vector<xr_string> names, std::vector<bool> oldVisibility, std::vector<bool> newVisibility)
	: m_names(std::move(names)), m_oldVisibility(std::move(oldVisibility)), m_newVisibility(std::move(newVisibility)) {
	VERIFY(m_names.size() == m_oldVisibility.size());
	VERIFY(m_names.size() == m_newVisibility.size());
}

void HideCommand::Undo() {
	for (size_t i = 0; i < m_names.size(); ++i) {
		CCustomObject* obj = Scene->FindObjectByName(m_names[i].c_str(), nullptr);
		if (obj && !obj->IsDeleted()) {
			obj->m_CO_Flags.set(CCustomObject::flVisible, m_oldVisibility[i]);
			obj->OnSynchronize();
		}
	}
}

void HideCommand::Redo() {
	for (size_t i = 0; i < m_names.size(); ++i) {
		CCustomObject* obj = Scene->FindObjectByName(m_names[i].c_str(), nullptr);
		if (obj && !obj->IsDeleted()) {
			obj->m_CO_Flags.set(CCustomObject::flVisible, m_newVisibility[i]);
			obj->OnSynchronize();
		}
	}
}

LockCommand::LockCommand(std::vector<xr_string> names, std::vector<bool> oldLocks, std::vector<bool> newLocks)
	: m_names(std::move(names)), m_oldLocks(std::move(oldLocks)), m_newLocks(std::move(newLocks)) {
	VERIFY(m_names.size() == m_oldLocks.size());
	VERIFY(m_names.size() == m_newLocks.size());
}

void LockCommand::Undo() {
	for (size_t i = 0; i < m_names.size(); ++i) {
		CCustomObject* obj = Scene->FindObjectByName(m_names[i].c_str(), nullptr);
		if (obj && !obj->IsDeleted()) {
			obj->m_CO_Flags.set(CCustomObject::flLocked, m_oldLocks[i]);
			obj->OnSynchronize();
		}
	}
}

void LockCommand::Redo() {
	for (size_t i = 0; i < m_names.size(); ++i) {
		CCustomObject* obj = Scene->FindObjectByName(m_names[i].c_str(), nullptr);
		if (obj && !obj->IsDeleted()) {
			obj->m_CO_Flags.set(CCustomObject::flLocked, m_newLocks[i]);
			obj->OnSynchronize();
		}
	}
}
