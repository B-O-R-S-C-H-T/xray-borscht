#include "stdafx.h"
#pragma hdrstop

#include "IM_Tree.h"

#include <imgui.h>

constexpr int MAX_EXPANDED_FOLDERS = 64;	// максимальное количество раскрытых папок в дереве

IM_Tree::ImTreeNode* IM_Tree::GetNode(LPCSTR path, bool must_exist) {
	ImTreeNode* node  = &root;
	int			count = _GetItemCount(path, separator);

	xr_string	name  = "";

	for (int i = 0; i < count; i++) {
		char str[256];
		_GetItem(path, i, str, separator);

		if (must_exist) {
			if (!node->child)
				return NULL;

			xr_map<xr_string, ImTreeNode>::iterator it = node->child->find(str);
			if (it == node->child->end())
				return NULL;

			node = &it->second;
		} else {
			if (!node->child)
				node->child = xr_new<xr_map<xr_string, ImTreeNode>>();

			if (i > 0)
				name += "\\";
			name += str;

			xr_map<xr_string, ImTreeNode>::iterator it = node->child->find(str);
			if (it == node->child->end()) {
				node	   = &(*node->child)[str];
				node->name = name.c_str();
			} else
				node = &it->second;
		}
	}

	return node;
}

inline xr_string GetBasename(const shared_str& full_path) {
	xr_string s	  = *full_path;
	size_t	  pos = s.rfind('\\');
	return (pos != xr_string::npos) ? s.substr(pos + 1) : s;
}

void IM_Tree::RenderNode(ImTreeNode& parent) {
	if (!parent.child)
		return;

	xr_vector<xr_map<xr_string, ImTreeNode>::iterator> folders;
	xr_vector<xr_map<xr_string, ImTreeNode>::iterator> files;

	for (auto it = parent.child->begin(); it != parent.child->end(); ++it) {
		if (it->second.item && !it->second.item->Visible())
			continue;

		if (it->second.child) // folder
			folders.push_back(it);
		else // file
			files.push_back(it);
	}

	std::sort(folders.begin(), folders.end(), [](const auto& a, const auto& b) { return a->first < b->first; });

	std::sort(files.begin(), files.end(), [&](const auto& a, const auto& b) { return NaturalCompare(a->first, b->first); });

	auto render_list = [&](auto& list)
	{
		for (auto map_it : list) {
			ImTreeNode& node   = map_it->second;
			bool		remove = false;
			bool		rename = false;

			ImGui::PushID(&node);

			if (renaming == &node) {
				ImGui::PushID(static_cast<void*>(&node));
				xr_string current_input(newname);
				if (current_input.empty() || current_input.find('\\') != xr_string::npos) {
					xr_string basename_only = GetBasename(node.name);
					strcpy(newname, basename_only.c_str());
				}
				ImGui::InputText("##rename_input", newname, sizeof(newname));
				ImGui::SameLine();
				bool ok = ImGui::Button("OK");
				ImGui::PopID();

				if ((GetAsyncKeyState(VK_RETURN) & 0x8000) || ok) {
					xr_string new_key(newname);
					if (parent.child->find(new_key) == parent.child->end()) {
						rename	 = true;
						renaming = nullptr;
					} else {
						ELog.Msg(mtError, "Element '%s' already exists", newname);
						renaming = nullptr;
					}
				}
			}
			// folder
			else if (node.child) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				if (node.selected)
					flags |= ImGuiTreeNodeFlags_Selected;
				if (full_expand || node.is_open)
					flags |= ImGuiTreeNodeFlags_DefaultOpen;

				ImGui::SetNextItemOpen(node.is_open, ImGuiCond_Always);

				bool open	 = ImGui::TreeNodeEx(map_it->first.c_str(), flags);
				node.is_open = open;

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					node.is_open = !node.is_open;
				}

				bool clicked = ImGui::IsItemClicked();

				if (ImGui::BeginPopupContextItem()) {
					ShowContextMenu(node, remove);
					ImGui::EndPopup();
				}

				if (clicked) {
					if (ImGui::GetIO().KeyShift && m_lastClickedNode) {
						SelectRange(m_lastClickedNode, &node);
					} else if (CheckmarkMode) {
						SelectNode(node, !node.selected);
						m_lastClickedNode = &node;
					} else {
						if (ImGui::GetIO().KeyCtrl)
							SelectNode(node, !node.selected);
						else {
							SelectAll(false);
							SelectNode(node, true);
						}
						m_lastClickedNode = &node;
					}

					if (node.item && !node.item->OnClickEvent.empty())
						node.item->OnClickEvent(node.item);
				}

				if (open) {
					RenderNode(node);
					ImGui::TreePop();
				}
			// file
			} else {
				xr_string display_name = GetBasename(node.name);

				bool color_pushed = false;
				if (node.item && node.item->prop_color != 0) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImU32(node.item->prop_color));
					color_pushed = true;
				}

				if (DrawBullets)
					ImGui::Bullet();

				bool clicked = ImGui::Selectable(display_name.c_str(), node.selected);

				if (color_pushed)
					ImGui::PopStyleColor();

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !OnItemRename.empty()) {
					SelectAll(false);
					SelectNode(node, true);
					strcpy(newname, GetBasename(node.name).c_str());
					renaming = &node;
				}

				if (ImGui::BeginPopupContextItem()) {
					ShowContextMenu(node, remove);
					ImGui::EndPopup();
				}

				if (DrawThumbnails && !OnDrawThumbnail.empty()) {
					if (!node.canvas) {
						const int w = 150, h = 150;
						node.canvas = xr_new<IM_Canvas>(w, h);
						HDC hdc		= node.canvas->BeginPaint();
						OnDrawThumbnail(*node.name, hdc, Irect().set(0, 0, w, h));
						node.canvas->EndPaint();
					}
					node.canvas->Render();
					if (ImGui::IsItemClicked())
						clicked = true;
				}

				if (clicked) {
					if (ImGui::GetIO().KeyShift && m_lastClickedNode) {
						SelectRange(m_lastClickedNode, &node);
					} else if (CheckmarkMode) {
						SelectNode(node, !node.selected);
						m_lastClickedNode = &node;
					} else {
						if (ImGui::GetIO().KeyCtrl)
							SelectNode(node, !node.selected);
						else {
							SelectAll(false);
							SelectNode(node, true);
						}
						m_lastClickedNode = &node;
					}

					if (node.item && !node.item->OnClickEvent.empty())
						node.item->OnClickEvent(node.item);
				}
			}

			ImGui::PopID();

			if (rename) {
				xr_string nm = map_it->first;
				RenameNode(parent, nm.c_str(), newname);
			}
		}
	};

	render_list(folders);
	render_list(files);
}

void IM_Tree::NotifySelectionChanged() {
	if (OnItemsFocused) {
		ListItemsVec vec;
		GetSelected(vec);
		OnItemsFocused(vec);
	}
}

void IM_Tree::SelectNode(ImTreeNode& node, bool select) {
	if (node.selected && !select) {
		selected.remove(&node);
		node.selected = false;
		if (!OnNodeSelected.empty())
			OnNodeSelected(node.name, node.value, select);
		if (renaming == &node)
			renaming = nullptr;
	} else if (!node.selected && select) {
		if (MaxSelectedCount >= 0 && GetSelectedCount() >= MaxSelectedCount) {
			return;
		}
		selected.push_back(&node);
		node.selected = true;

		// so much callbacks ...
		if (!OnNodeSelected.empty())
			OnNodeSelected(node.name, node.value, select);
		if (node.item && !node.item->OnItemFocused.empty())
			node.item->OnItemFocused(node.item);
		if (OnItemFocused)
			OnItemFocused(nullptr);
	}

	if (!m_InBatchSelection)
		NotifySelectionChanged();
}

bool IM_Tree::RenameNode(ImTreeNode& parent, LPCSTR oldname, LPCSTR newname) {
	if (!parent.child || parent.child->find(oldname) == parent.child->end())
		return false;

	xr_string new_key;
	xr_string new_full_path;

	if (strchr(newname, '\\') || strchr(newname, '/')) {
		new_full_path = newname;
		size_t pos	  = new_full_path.rfind('\\');
		new_key		  = (pos != xr_string::npos) ? new_full_path.substr(pos + 1) : new_full_path;
	} else {
		new_key			   = newname;
		xr_string old_full = *(*parent.child)[oldname].name;
		size_t	  pos	   = old_full.rfind('\\');
		if (pos != xr_string::npos)
			new_full_path = old_full.substr(0, pos + 1) + newname;
		else
			new_full_path = newname;
	}

	if (parent.child->find(new_key) != parent.child->end())
		return false;

	ImTreeNode& n = (*parent.child)[oldname];
	SelectNode(n, false);

	(*parent.child)[new_key] = n;
	ImTreeNode& n2			 = (*parent.child)[new_key];

	n2.name					 = new_full_path.c_str();
	if (n2.item)
		n2.item->SetName(new_key.c_str());

	if (!OnItemRename.empty())
		OnItemRename(n.name.c_str(), new_full_path.c_str(), n2.child ? TYPE_FOLDER : TYPE_OBJECT);

	n.ClearNotFree();
	parent.child->erase(oldname);

	SelectNode(n2, true);
	return true;
}

void IM_Tree::ShowContextMenu(ImTreeNode& node, bool& removed) {
	if (!OnItemRename.empty() && ImGui::MenuItem("Rename##tree_context_rename")) {
		SelectAll(false);
		SelectNode(node, true);
		strcpy(newname, GetBasename(node.name).c_str());
		renaming = &node;
	}

	if (!OnItemRemove.empty() && ImGui::MenuItem("Remove##tree_context_remove")) {
		if (GetSelectedCount() > 1 && node.selected) {
			ListItemsVec sel;
			GetSelected(sel);
			for (ListItem* item : sel) {
				if (item && item->Visible()) {
					bool res = false;
					OnItemRemove(item->Key(), (EItemType)item->Type(), res);
				}
			}
		} else {
			OnItemRemove(*node.name, node.child ? TYPE_FOLDER : TYPE_OBJECT, removed);
		}
	}
}

void IM_Tree::GetItems(ImTreeNode& node, ListItemsVec& result) {
	if (!node.child)
		return;

	xr_map<xr_string, ImTreeNode>::iterator it, end;
	for (it = node.child->begin(), end = node.child->end(); it != end; it++) {
		if (it->second.item)
			result.push_back(it->second.item);
		GetItems(it->second, result);
	}
}

void IM_Tree::CollectLinearOrder(ImTreeNode* node, std::vector<ImTreeNode*>& out, bool onlyRootLevelFiles) {
	if (!node)
		return;

	if (!node->child) {
		out.push_back(node);
		return;
	}

	if (onlyRootLevelFiles && node == &root) {
		for (auto& pair : *node->child) {
			if (!pair.second.child)
				out.push_back(&pair.second);
		}
		return;
	}

	bool should_recurse = (node == &root) || node->is_open;

	if (should_recurse) {
		for (auto& pair : *node->child) {
			CollectLinearOrder(&pair.second, out, onlyRootLevelFiles);
		}
	}
}

void IM_Tree::SelectRange(ImTreeNode* from, ImTreeNode* to) {
	if (!from || !to)
		return;

	bool isRootLevel = (strchr(from->name.c_str(), '\\') == nullptr) && (strchr(to->name.c_str(), '\\') == nullptr);

	std::vector<ImTreeNode*> linear;
	CollectLinearOrder(&root, linear, isRootLevel);

	auto it_from = std::find(linear.begin(), linear.end(), from);
	auto it_to	 = std::find(linear.begin(), linear.end(), to);

	if (it_from == linear.end() || it_to == linear.end())
		return;

	if (it_from > it_to)
		std::swap(it_from, it_to);

	m_InBatchSelection = true;

	for (auto it = it_from; it <= it_to; ++it) {
		if (!(*it)->selected) {
			if (MaxSelectedCount >= 0 && GetSelectedCount() >= MaxSelectedCount) {
				break;
			}
			SelectNode(**it, true);
		}
	}

	m_InBatchSelection = false;
	NotifySelectionChanged();
}

void IM_Tree::Add(LPCSTR path, LPCSTR value) {
	if (path == NULL)
		DebugBreak();
	ImTreeNode* node = GetNode(path, false);
	node->name		 = path;
	node->value		 = value;
}

void IM_Tree::Add(LPCSTR path, ListItem* item) {
	if (path == NULL)
		DebugBreak();
	ImTreeNode* node = GetNode(path, false);
	node->name		 = path;
	node->item		 = item;
}

void IM_Tree::Clear() {
	ClearNode(root);
	selected.clear();
	renaming = NULL;
}

void IM_Tree::Select(LPCSTR path, bool select) {
	ImTreeNode* node = GetNode(path, true);
	if (!node)
		return;

	m_InBatchSelection = true;
	SelectNode(*node, select);
	m_InBatchSelection = false;
	NotifySelectionChanged();
}

void IM_Tree::SelectAll(bool select) {
	if (select) {
		if (!multiselect)
			return;
		return;
		// todo
	}

	xr_list<ImTreeNode*> sel = selected; // copy
	m_InBatchSelection		 = true;

	for (auto it = sel.begin(); it != sel.end(); ++it)
		SelectNode(**it, false);

	m_InBatchSelection = false;
	NotifySelectionChanged();

	m_lastClickedNode = nullptr;
	renaming		  = nullptr;
}

shared_str IM_Tree::GetSelected() {
	if (selected.size())
		return selected.front()->value;
	else
		return NULL;
}

void IM_Tree::GetSelected(xr_vector<shared_str>& result) {
	xr_list<ImTreeNode*>::iterator it;
	for (it = selected.begin(); it != selected.end(); it++) {
		result.push_back((*it)->value);
	}
}

shared_str IM_Tree::GetSelectedPath() {
	if (selected.size())
		return selected.front()->name;
	else
		return NULL;
}

void IM_Tree::GetSelectedPaths(xr_vector<shared_str>& result) {
	xr_list<ImTreeNode*>::iterator it;
	for (it = selected.begin(); it != selected.end(); it++) {
		result.push_back((*it)->name);
	}
}

int IM_Tree::GetSelectedCount() {
	return selected.size();
}

void IM_Tree::AssignItems(ListItemsVec& items, bool full_expand) {
	if (m_HasPendingUpdate) {
		m_PendingItems		= items;
		m_PendingFullExpand = full_expand;
		return;
	}

	m_PendingItems		= items;
	m_HasPendingUpdate	= true;
	m_PendingFullExpand = full_expand;
}

void IM_Tree::GetSelected(ListItemsVec& result) {
	for (xr_list<ImTreeNode*>::iterator it = selected.begin(); it != selected.end(); it++)
		if ((*it)->item)
			result.push_back((*it)->item);
}

void IM_Tree::GetItems(ListItemsVec& result) {
	GetItems(root, result);
}

bool IM_Tree::NaturalCompare(const xr_string& lhs, const xr_string& rhs) const {
	size_t i = 0;
	size_t j = 0;

	while (i < lhs.size() && j < rhs.size()) {
		if (lhs[i] >= '0' && lhs[i] <= '9' && rhs[j] >= '0' && rhs[j] <= '9') {
			size_t num_lhs = 0;
			size_t num_rhs = 0;

			while (i < lhs.size() && lhs[i] >= '0' && lhs[i] <= '9') {
				num_lhs = num_lhs * 10 + (lhs[i] - '0');
				++i;
			}
			while (j < rhs.size() && rhs[j] >= '0' && rhs[j] <= '9') {
				num_rhs = num_rhs * 10 + (rhs[j] - '0');
				++j;
			}

			if (num_lhs != num_rhs)
				return num_lhs < num_rhs;
			continue;
		}

		if (lhs[i] != rhs[j])
			return lhs[i] < rhs[j];

		++i;
		++j;
	}

	return lhs.size() < rhs.size();
}

void IM_Tree::InvalidateCache(const shared_str& key) {
	if (!key)
		return;

	ImTreeNode* node = GetNode(key.c_str(), true);
	if (node && node->canvas) {
		xr_delete(node->canvas);
		node->canvas = nullptr;
	}
}

void IM_Tree::Render() {
	if (m_HasPendingUpdate) {
		Clear();

		for (ListItem* item : m_PendingItems) {
			if (item)
				Add(item->Key(), item);
		}

		full_expand = m_PendingFullExpand;

		m_PendingItems.clear();
		m_HasPendingUpdate = false;
	}

	RenderNode(root);

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive()) {
		SelectAll(false);
	}
}

void IM_Tree::SaveExpandedState(IM_Storage& storage, const char* key) {
	xr_string result;
	result.reserve(8192);

	int count = 0;

	std::function<void(ImTreeNode*)> collect = [&](ImTreeNode* node)
	{
		if (!node || !node->child)
			return;

		if (node->is_open && node != &root) {
			if (count >= MAX_EXPANDED_FOLDERS)
				return;

			if (!result.empty())
				result += '|';
			result += *node->name;
			count++;
		}

		for (auto& pair : *node->child)
			collect(&pair.second);
	};

	collect(&root);
	storage.PutString(key, result.c_str());
}

void IM_Tree::RestoreExpandedState(IM_Storage& storage, const char* key) {
	xr_string data = storage.GetString(key, "");
	if (data.empty())
		return;

	xr_vector<xr_string> paths;
	_SequenceToList(paths, data.c_str(), '|');

	for (const auto& path : paths) {
		ImTreeNode* node = GetNode(path.c_str(), true);
		if (node && node->child)
			node->is_open = true;
	}
}

void IM_Tree::CollapseAll() {
	std::function<void(ImTreeNode*)> coll = [&](ImTreeNode* node)
	{
		if (!node || !node->child)
			return;
		if (node != &root)
			node->is_open = false;
		for (auto& p : *node->child)
			coll(&p.second);
	};
	coll(&root);
}

void IM_Tree::CollapseAllExceptSelection() {
	if (selected.empty()) {
		CollapseAll();
		return;
	}

	std::set<xr_string> keep;
	for (auto* node : selected) {
		if (!node)
			continue;

		xr_string p = *node->name;
		while (!p.empty()) {
			keep.insert(p);
			size_t pos = p.rfind('\\');
			if (pos == xr_string::npos)
				break;
			p = p.substr(0, pos);
		}
	}

	std::function<void(ImTreeNode*)> process = [&](ImTreeNode* node)
	{
		if (!node || !node->child)
			return;

		// у root нет валидного имени
		if (node != &root) {
			xr_string nodePath = *node->name;
			if (keep.find(nodePath) == keep.end()) {
				node->is_open = false;
			}
		}

		for (auto& pr : *node->child) {
			process(&pr.second);
		}
	};

	process(&root);
}

void IM_Tree::ExpandAllDescendantsOfSelection() {
	for (auto* sel : selected) {
		if (!sel->child)
			continue; // не папка

		std::function<void(ImTreeNode*)> exp = [&](ImTreeNode* node)
		{
			if (!node)
				return;
			if (node->child)
				node->is_open = true;

			if (node->child) {
				for (auto& pr : *node->child)
					exp(&pr.second);
			}
		};

		exp(sel);
	}
}
