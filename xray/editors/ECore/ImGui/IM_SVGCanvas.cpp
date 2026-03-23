#include "stdafx.h"
#pragma hdrstop
#include "IM_SVGCanvas.h"

#include "../Editor/device.h"
#include "inc/thorvg.h"
#include <imgui.h>

xr_map<shared_str, IM_SVGCanvas*> IM_SVGCanvas::s_cache;
bool							  IM_SVGCanvas::s_initialized = false;

IM_SVGCanvas::IM_SVGCanvas(int width, int height) : IM_Canvas(width, height) {
	if (width <= 0 || height <= 0) {
		ELog.Msg(mtError, "IM_SVGCanvas: invalid size %dx%d", width, height);
		return;
	}
	m_tvgBuffer.resize(static_cast<size_t>(width) * height);
}

IM_SVGCanvas::~IM_SVGCanvas() {}

void IM_SVGCanvas::FreeTvgResources() {
	if (m_picture) {
		m_picture->unref(true);
		m_picture = nullptr;
	}
}

void IM_SVGCanvas::RebuildTextureAndBuffer() {
	if (!HW.pDevice) {
		ELog.Msg(mtError, "IM_SVGCanvas: HW.pDevice is null");
		return;
	}

	m_tvgBuffer.resize(static_cast<size_t>(m_width) * m_height);

	if (m_tvgCanvas) {
		delete m_tvgCanvas;
		m_tvgCanvas = nullptr;
	}

	if (m_picture) {
		m_picture->size(static_cast<float>(m_width), static_cast<float>(m_height));
		m_tvgCanvas = tvg::SwCanvas::gen(tvg::EngineOption::Default);
		if (!m_tvgCanvas) {
			ELog.Msg(mtError, "IM_SVGCanvas: failed to create SwCanvas");
			return;
		}
		m_tvgCanvas->target(m_tvgBuffer.data(), m_width, m_width, m_height, tvg::ColorSpace::ARGB8888);
		m_tvgCanvas->add(m_picture);
		m_tvgCanvas->draw();
		m_tvgCanvas->sync();
	}

	DestroyTexture();
	CreateTexture(D3DFMT_A8R8G8B8);

	if (!m_texture) {
		ELog.Msg(mtError, "IM_SVGCanvas: failed to create texture");
		return;
	}

	IDirect3DTexture9* tex = nullptr;
	CHK_DX(HW.pDevice->CreateTexture(m_width, m_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &tex, nullptr));

	RECT		   r  = {0, 0, m_width, m_height};
	D3DLOCKED_RECT lr = {0};
	CHK_DX(tex->LockRect(0, &lr, &r, 0));
	memcpy(lr.pBits, m_tvgBuffer.data(), static_cast<size_t>(m_width) * m_height * 4);
	CHK_DX(tex->UnlockRect(0));

	IDirect3DSurface9* S = nullptr;
	IDirect3DSurface9* D = nullptr;
	CHK_DX(tex->GetSurfaceLevel(0, &S));
	CHK_DX(m_texture->pSurface->GetSurfaceLevel(0, &D));
	CHK_DX(HW.pDevice->UpdateSurface(S, nullptr, D, nullptr));

	D->Release();
	S->Release();
	tex->Release();
}

bool IM_SVGCanvas::Load(const char* svgPath) {
	if (!svgPath) {
		ELog.Msg(mtError, "IM_SVGCanvas::Load: null path");
		return false;
	}

	FreeTvgResources();

	m_picture = tvg::Picture::gen();
	tvg::Result res = m_picture->load(svgPath);
	if (res != tvg::Result::Success) {
		ELog.Msg(mtError, "IM_SVGCanvas: failed to load SVG '%s'", svgPath);
		m_picture->unref(true);
		m_picture = nullptr;
		return false;
	}

	RebuildTextureAndBuffer();
	return true;
}

void IM_SVGCanvas::Resize(int width, int height) {
	if (width <= 0 || height <= 0) {
		ELog.Msg(mtError, "IM_SVGCanvas::Resize: invalid size %dx%d", width, height);
		return;
	}
	if (width == m_width && height == m_height)
		return;

	m_width	 = width;
	m_height = height;
	RebuildTextureAndBuffer();
}

IM_SVGCanvas* IM_SVGCanvas::GetIcon(const char* svgPath, int size) {
	if (!svgPath || size <= 0) {
		ELog.Msg(mtError, "IM_SVGCanvas::GetIcon: invalid arguments");
		return nullptr;
	}

	char keybuf[512];
	sprintf(keybuf, "%s_%d", svgPath, size);
	shared_str key = keybuf;

	auto it  = s_cache.find(key);
	if (it != s_cache.end()) {
		return it->second;
	}

	IM_SVGCanvas* icon = xr_new<IM_SVGCanvas>(size, size);
	if (!icon) {
		ELog.Msg(mtError, "IM_SVGCanvas::GetIcon: failed to allocate");
		return nullptr;
	}

	if (icon->Load(svgPath)) {
		s_cache[key] = icon;
		return icon;
	}

	ELog.Msg(mtError, "IM_SVGCanvas::GetIcon: failed to load '%s'", svgPath);
	return nullptr;
}

void IM_SVGCanvas::InitializeThorVG() {
	if (!s_initialized) {
		tvg::Initializer::init(0);
		s_initialized = true;
	}
}
void IM_SVGCanvas::TerminateThorVG() {
	if (s_initialized) {
		tvg::Initializer::term();
		s_initialized = false;
	}
}

void IM_SVGCanvas::ClearCache() {
	if (s_cache.empty())
		return;

	for (auto& p : s_cache) {
		if (p.second) p.second->FreeTvgResources();
	}
	s_cache.clear();
}

void IM_SVGCanvas::Render() {
	IM_Canvas::Render();
}
