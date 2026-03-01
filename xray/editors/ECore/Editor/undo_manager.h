///////////////////////////////////////////////////////////////////////////////
// undo_manager.h
// Командно-ориентированная система отмены/повтора действий
// (C) 2026 Проект Б.О.Р.Ж.Т.Ч. (xray-engine.org)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "CustomObject.h"

class ICommand {
public:
	virtual ~ICommand() = default;
	virtual void Undo() = 0;
	virtual void Redo() = 0;
	virtual void ReleaseResources() {}
};

class UndoManager {
public:
	void Push(std::unique_ptr<ICommand> cmd);
	void Undo();
	void Redo();
	void Clear();
	bool CanUndo() const { return !m_undoStack.empty(); }
	bool CanRedo() const { return !m_redoStack.empty(); }

private:
	std::deque<std::unique_ptr<ICommand>> m_undoStack;
	std::deque<std::unique_ptr<ICommand>> m_redoStack;
};

class MoveCommand : public ICommand {
public:
	MoveCommand(const std::vector<CCustomObject*>& objects,
				const std::vector<Fvector>&		   oldPositions,
				const std::vector<Fvector>&		   newPositions,
				const Fvector&					   oldCamPos,
				const Fvector&					   oldCamHPB);

	void Undo() override;
	void Redo() override;
	void ReleaseResources() override {}

private:
	std::vector<CCustomObject*> m_objects;
	std::vector<Fvector>		m_oldPositions;
	std::vector<Fvector>		m_newPositions;

	Fvector						m_oldCamPos;
	Fvector						m_oldCamHPB;
	Fvector						m_newCamPos;
	Fvector						m_newCamHPB;
};

class ScaleCommand : public ICommand {
public:
	ScaleCommand(const std::vector<CCustomObject*>& objects,
				 const std::vector<Fvector>&		oldScales,
				 const std::vector<Fvector>&		newScales,
				 const Fvector&						oldCamPos,
				 const Fvector&						oldCamHPB);

	void Undo() override;
	void Redo() override;
	void ReleaseResources() override {}

private:
	std::vector<CCustomObject*> m_objects;
	std::vector<Fvector>		m_oldScales;
	std::vector<Fvector>		m_newScales;

	Fvector						m_oldCamPos;
	Fvector						m_oldCamHPB;
	Fvector						m_newCamPos;
	Fvector						m_newCamHPB;
};

class RotateCommand : public ICommand {
public:
	RotateCommand(const std::vector<CCustomObject*>& objects,
				  const std::vector<Fvector>&		 oldRotations,
				  const std::vector<Fvector>&		 newRotations,
				  const Fvector&					 oldCamPos,
				  const Fvector&					 oldCamHPB);

	void Undo() override;
	void Redo() override;
	void ReleaseResources() override {}

private:
	std::vector<CCustomObject*> m_objects;
	std::vector<Fvector>		m_oldRotations;
	std::vector<Fvector>		m_newRotations;

	Fvector						m_oldCamPos;
	Fvector						m_oldCamHPB;
	Fvector						m_newCamPos;
	Fvector						m_newCamHPB;
};

class PasteCommand : public ICommand {
public:
	explicit PasteCommand(std::vector<CCustomObject*> added);
	void Undo() override;
	void Redo() override;
	~PasteCommand() override;

private:
	std::vector<CCustomObject*> m_added;
};

class DuplicateCommand : public ICommand {
public:
	explicit DuplicateCommand(std::vector<CCustomObject*> added);
	void Undo() override;
	void Redo() override;
	~DuplicateCommand() override;

private:
	std::vector<CCustomObject*> m_added;
};

class CutCommand : public ICommand {
public:
	explicit CutCommand(std::vector<CCustomObject*> objects);
	void Undo() override;
	void Redo() override;
	~CutCommand() override;

private:
	std::vector<CCustomObject*> m_objects;
	bool						m_cut = false;
};

class AddObjectCommand : public ICommand {
public:
	explicit AddObjectCommand(CCustomObject* obj);
	void Undo() override;
	void Redo() override;
	~AddObjectCommand() override;

private:
	CCustomObject* m_object;
};

class RemoveObjectsCommand : public ICommand {
	std::vector<CCustomObject*> m_objects;

public:
	explicit RemoveObjectsCommand(std::vector<CCustomObject*> objects);
	void Undo() override;
	void Redo() override;
	void ReleaseResources() override;
	~RemoveObjectsCommand() override = default;
};

class RenameCommand : public ICommand {
public:
	RenameCommand(std::vector<CCustomObject*> objects, std::vector<xr_string> oldNames, std::vector<xr_string> newNames);
	void Undo() override;
	void Redo() override;
	void ReleaseResources() override {}

private:
	std::vector<CCustomObject*> m_objects;
	std::vector<xr_string>		m_oldNames;
	std::vector<xr_string>		m_newNames;
};

using AddFromSelectionCommand = PasteCommand;

class HideCommand : public ICommand {
public:
	HideCommand(std::vector<xr_string> names, std::vector<bool> oldVisibility, std::vector<bool> newVisibility);
	void Undo() override;
	void Redo() override;
	void ReleaseResources() override {}

private:
	std::vector<xr_string> m_names;
	std::vector<bool>	   m_oldVisibility;
	std::vector<bool>	   m_newVisibility;
};

class LockCommand : public ICommand {
public:
	LockCommand(std::vector<xr_string> names, std::vector<bool> oldLocks, std::vector<bool> newLocks);
	void Undo() override;
	void Redo() override;
	void ReleaseResources() override {}

private:
	std::vector<xr_string> m_names;
	std::vector<bool>	   m_oldLocks;
	std::vector<bool>	   m_newLocks;
};
