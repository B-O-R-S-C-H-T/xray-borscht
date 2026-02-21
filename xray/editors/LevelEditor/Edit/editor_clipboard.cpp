#include "stdafx.h"

#include <editors/ECore/Editor/editor_clipboard.h>

#include "CustomObject.h"
#include "scene.h"

void EditorClipboard::SaveToStream(ObjClassID classfilter, EScene* scene, IWriter& writer) {
	scene->SaveSelectionToStream(classfilter, writer);
}

void EditorClipboard::LoadFromStream(EScene* scene, IReader& reader) {
	scene->LoadSelectionFromStream(reader);
}

void EditorClipboard::Copy(ObjClassID classfilter, EScene* scene) {
	m_data.clear();
	SaveToStream(classfilter, scene, m_data);
}

void EditorClipboard::Paste(EScene* scene) {
	if (!HasData())
		return;

	IReader reader(m_data.pointer(), m_data.size());
	LoadFromStream(scene, reader);
}

void EditorClipboard::Duplicate(ObjClassID classfilter, EScene* scene) {
	m_data.clear();
	SaveToStream(classfilter, scene, m_data);

	IReader reader(m_data.pointer(), m_data.size());
	LoadFromStream(scene, reader);
}

bool EditorClipboard::HasData() const {
	return m_data.size() > 0;
}
