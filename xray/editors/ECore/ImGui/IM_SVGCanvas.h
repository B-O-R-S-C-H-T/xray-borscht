#pragma once

#include <inc/thorvg.h>
#include <vector>

#include "IM_Canvas.h"

class ECORE_API IM_SVGCanvas : public IM_Canvas {
protected:
	tvg::SwCanvas*		  m_tvgCanvas{nullptr};
	tvg::Picture*		  m_picture{nullptr};
	std::vector<uint32_t> m_tvgBuffer;

private:
	static xr_map<shared_str, IM_SVGCanvas*> s_cache;
	static bool								 s_initialized;
	static xr_set<shared_str>				 s_failed_loads;

	static int								 s_current_icon_size;
	static bool								 s_need_full_rebuild;

	void									 FreeTvgResources();
	void									 RebuildTextureAndBuffer();

public:
	IM_SVGCanvas(int width, int height);
	~IM_SVGCanvas();

	bool				 Load(const char* svgPath);
	void				 Resize(int width, int height);

	static IM_SVGCanvas* GetIcon(const char* svgPath, int size = 32);
	static void			 InitializeThorVG();
	static void			 TerminateThorVG();
	static void			 ClearCache();

	static void			 RequestResize(int size);
	static void			 ProcessPendingRebuild();
	static void			 ForceRebuild();

	virtual void		 Render();
};
