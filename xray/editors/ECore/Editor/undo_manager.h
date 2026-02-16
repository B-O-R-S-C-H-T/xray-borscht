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

private:
	std::vector<CCustomObject*> m_objects;
	std::vector<Fvector>		m_oldRotations;
	std::vector<Fvector>		m_newRotations;

	Fvector						m_oldCamPos;
	Fvector						m_oldCamHPB;
	Fvector						m_newCamPos;
	Fvector						m_newCamHPB;
};
