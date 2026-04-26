#ifndef IM_TreeH
#define IM_TreeH

#include <functional>
#include <xrServerEntities/ItemListTypes.h>
#include <xrServerEntities/xrEProps.h>

#include "IM_Canvas.h"
#include "IM_Window.h"

typedef fastdelegate::FastDelegate<void(shared_str, shared_str, bool)> IM_TOnNodeSelected;
typedef fastdelegate::FastDelegate<void(LPCSTR, HDC, const Irect&)>	   IM_TOnDrawThumbnail;

class ECORE_API IM_Tree : public IM_Window {
private:
	char separator;

	struct ImTreeNode {
		xr_map<xr_string, ImTreeNode>* child;
		bool						   selected;
		bool						   is_open;

		shared_str name;
		shared_str value;
		ListItem*  item;

		IM_Canvas* canvas;

		ImTreeNode() : child(NULL), selected(false), is_open(false), name(NULL), value(NULL), item(NULL), canvas(NULL) {}

		void Clear() {
			if (child)
				xr_delete(child);
			if (item)
				xr_delete(item);
			if (canvas)
				xr_delete(canvas);
		}

		void ClearNotFree() {
			child  = NULL;
			item   = NULL;
			canvas = NULL;
		}

		~ImTreeNode() { Clear(); }
	};

	char				 newname[512];
	ImTreeNode*			 renaming;

	ImTreeNode			 root;
	xr_list<ImTreeNode*> selected;

	ImTreeNode*			 m_lastClickedNode = nullptr;

	bool				 full_expand		 = false;

	ListItemsVec		 m_PendingItems;
	bool				 m_HasPendingUpdate  = false;
	bool				 m_PendingFullExpand = false;

	bool				 m_InBatchSelection  = false;

	ImTreeNode*			 GetNode(LPCSTR path, bool must_exist);
	void				 RenderNode(ImTreeNode& node);
	void				 ClearNode(ImTreeNode& node) { node.Clear(); }
	void				 SelectNode(ImTreeNode& node, bool select = true);
	bool				 RenameNode(ImTreeNode& parent, LPCSTR oldname, LPCSTR newname);
	void				 ShowContextMenu(ImTreeNode& node, bool& removed);
	void				 GetItems(ImTreeNode& node, ListItemsVec& result);
	void				 SelectRange(ImTreeNode* from, ImTreeNode* to);
	void				 CollectLinearOrder(ImTreeNode* node, std::vector<ImTreeNode*>& out, bool onlyRootLevelFiles = false);
	bool				 NaturalCompare(const xr_string& lhs, const xr_string& rhs) const;
	void				 NotifySelectionChanged();

	std::function<void(const shared_str&)> m_OnInvalidateCache;

public: // properties
	bool				multiselect;
	bool				DrawBullets;
	bool				CheckmarkMode;
	bool				DrawThumbnails;
	int					MaxSelectedCount;
	IM_TOnNodeSelected	OnNodeSelected;
	IM_TOnDrawThumbnail OnDrawThumbnail;

public:
	IM_Tree(char folder_separator = '\\', bool allow_multiselect = false, bool draw_bullets = false)
		: separator(folder_separator), multiselect(allow_multiselect), renaming(nullptr), DrawBullets(draw_bullets), CheckmarkMode(false),
		  DrawThumbnails(false), MaxSelectedCount(-1), OnNodeSelected(nullptr), OnDrawThumbnail(nullptr), full_expand(false), m_HasPendingUpdate(false),
		  m_PendingFullExpand(false), newname{} { root.is_open = true; }

	~IM_Tree() { Clear(); }

	void			  Add(LPCSTR path, LPCSTR value);
	void			  Add(LPCSTR path, ListItem* item);
	void			  Clear();

	void			  Select(LPCSTR path, bool select = true);
	void			  SelectAll(bool select = true);

	shared_str		  GetSelected();
	void			  GetSelected(xr_vector<shared_str>& result); // for multiselect tree

	shared_str		  GetSelectedPath();
	void			  GetSelectedPaths(xr_vector<shared_str>& result);

	int				  GetSelectedCount();

	// for compatibility with xrEProps item list
	TOnILItemsFocused OnItemsFocused;
	TOnILItemFocused  OnItemFocused;
	TOnItemRename	  OnItemRename;
	TOnItemRemove	  OnItemRemove;

	void			  AssignItems(ListItemsVec& items, bool full_expand);
	void			  GetSelected(ListItemsVec& result);

	void			  GetItems(ListItemsVec& result);

	void			  SetCacheInvalidator(std::function<void(const shared_str&)> func) { m_OnInvalidateCache = std::move(func); }
	void			  InvalidateCache(const shared_str& key);

	virtual void	  Render();
};

#endif
