#pragma once

#include <editors/LevelEditor/Edit/ESceneClassList.h>
#include <xrCore/FS.h>

#include "xrCore/xrMemory.h"

class EScene;

class EditorClipboard {
public:
	EditorClipboard()  = default;
	~EditorClipboard() = default;

	void Copy(ObjClassID classfilter, EScene* scene);
	void Paste(EScene* scene);
	void Duplicate(ObjClassID classfilter, EScene* scene);

	bool HasData() const;

private:
	CMemoryWriter m_data;

	void		  SaveToStream(ObjClassID classfilter, EScene* scene, IWriter& writer);
	void		  LoadFromStream(EScene* scene, IReader& reader);
};
