#include <assert.h>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <Graphics/Context.h>
#include <Graphics/Parameters.h>
#include "Config.h"
#include "DisplayWindow.h"
#include "Textures.h"
#include "RDP.h"
#include "RSP.h"
#include "VI.h"
#include "FrameBuffer.h"
#include "TexrectDrawer.h"

using namespace graphics;

TexrectDrawer::TexrectDrawer()
: m_numRects(0)
, m_otherMode(0)
, m_mux(0)
, m_ulx(0)
, m_lrx(0)
, m_uly(0)
, m_lry(0)
, m_ulx_i(0)
, m_uly_i(0)
, m_lry_i(0)
, m_Z(0)
, m_max_lrx(0)
, m_max_lry(0)
, m_scissor(gDPScissor())
, m_pTexture(nullptr)
, m_pBuffer(nullptr)
{}

void TexrectDrawer::init()
{
	if (!gfxContext)
		return;

	const FramebufferTextureFormats & fbTexFormats = gfxContext.getFramebufferTextureFormats();

	m_FBO = gfxContext.createFramebuffer();

	m_pTexture = textureCache().addFrameBufferTexture(textureTarget::TEXTURE_2D);
	m_pTexture->format = G_IM_FMT_RGBA;
	m_pTexture->clampS = 1;
	m_pTexture->clampT = 1;
	m_pTexture->frameBufferTexture = CachedTexture::fbOneSample;
	m_pTexture->maskS = 0;
	m_pTexture->maskT = 0;
	m_pTexture->mirrorS = 0;
	m_pTexture->mirrorT = 0;
	m_pTexture->width  = 640;
	m_pTexture->height = 580;
	m_pTexture->textureBytes = m_pTexture->width * m_pTexture->height * fbTexFormats.colorFormatBytes;

	Context::InitTextureParams initParams;
	initParams.handle = m_pTexture->name;
	initParams.textureUnitIndex = textureIndices::Tex[0];
	initParams.width = m_pTexture->width;
	initParams.height = m_pTexture->height;
	initParams.internalFormat = fbTexFormats.colorInternalFormat;
	initParams.format = fbTexFormats.colorFormat;
	initParams.dataType = fbTexFormats.colorType;
	gfxContext.init2DTexture(initParams);

	Context::TexParameters texParams;
	texParams.handle = m_pTexture->name;
	texParams.target = textureTarget::TEXTURE_2D;
	texParams.textureUnitIndex = textureIndices::Tex[0];
	texParams.minFilter = textureParameters::FILTER_LINEAR;
	texParams.magFilter = textureParameters::FILTER_LINEAR;
	gfxContext.setTextureParameters(texParams);


	Context::FrameBufferRenderTarget bufTarget;
	bufTarget.bufferHandle = m_FBO;
	bufTarget.bufferTarget = bufferTarget::DRAW_FRAMEBUFFER;
	bufTarget.attachment = bufferAttachment::COLOR_ATTACHMENT0;
	bufTarget.textureTarget = textureTarget::TEXTURE_2D;
	bufTarget.textureHandle = m_pTexture->name;
	gfxContext.addFrameBufferRenderTarget(bufTarget);

	// check if everything is OK
	assert(!gfxContext.isFramebufferError());
	gfxContext.bindFramebuffer(bufferTarget::DRAW_FRAMEBUFFER, ObjectHandle::defaultFramebuffer);

	m_programTex.reset(gfxContext.createTexrectDrawerDrawShader());
	m_programClear.reset(gfxContext.createTexrectDrawerClearShader());
	m_programTex->setTextureSize(m_pTexture->width, m_pTexture->height);

	m_vecRectCoords.reserve(256);
}

void TexrectDrawer::destroy()
{
	gfxContext.deleteFramebuffer(m_FBO);
	if (m_pTexture != nullptr) {
		textureCache().removeFrameBufferTexture(m_pTexture);
		m_pTexture = nullptr;
	}
	m_programTex.reset();
	m_programClear.reset();
}

void TexrectDrawer::_setViewport() const
{
	dwnd().getDrawer()._updateViewport(nullptr, 1.0f);
}

void TexrectDrawer::_setDrawBuffer()
{
	if (m_pBuffer != nullptr)
		gfxContext.bindFramebuffer(bufferTarget::DRAW_FRAMEBUFFER, m_pBuffer->m_FBO);
	else
		frameBufferList().setCurrentDrawBuffer();
}

TexrectDrawer::iRect TexrectDrawer::_getiRect(word w0, word w1) const
{
	iRect rect;
	rect.ulx = _SHIFTR(w1, 12, 12);
	rect.uly = _SHIFTR(w1, 0, 12);
	rect.lrx = _SHIFTR(w0, 12, 12);
	rect.lry = _SHIFTR(w0, 0, 12);
	return rect;
}

#define COMPARE_COORDS(a, b) std::abs(a - b) <= 4

bool TexrectDrawer::_lookAhead(bool _checkCoordinates) const
{
	if (config.graphics2D.enableNativeResTexrects != Config::NativeResTexrectsMode::ntOptimized)
		return true;
	if (RSP.LLE)
		return true;
	switch (GBI.getMicrocodeType()) {
	case Turbo3D:
	case T3DUX:
	case F5Rogue:
	case F5Indi_Naboo:
		return true;
	}

	auto sideBySide = [&](word pc) ->bool {
		const word w0 = *(word*)&RDRAM[pc];
		const word w1 = *(word*)&RDRAM[pc + sizeof(word)];
		const iRect nextRect = _getiRect(w0, w1);

		if (COMPARE_COORDS(m_curRect.ulx, nextRect.ulx)) {
			bool sbs = COMPARE_COORDS(m_curRect.lry, nextRect.uly);
			sbs |= COMPARE_COORDS(m_curRect.uly, nextRect.lry);
			return sbs;
		}
		if (COMPARE_COORDS(m_curRect.uly, nextRect.uly)) {
			bool sbs = COMPARE_COORDS(m_curRect.lrx, nextRect.ulx);
			sbs |= COMPARE_COORDS(m_curRect.ulx, nextRect.lrx);
			return sbs;
		}
		return false;
	};

	u32 pc = RSP.PC[RSP.PCi];
	while (true) {
		switch (_SHIFTR(*(u32*)&RDRAM[pc], 24, 8)) {
		case G_RDPLOADSYNC:
		case G_RDPPIPESYNC:
		case G_RDPTILESYNC:
		case G_LOADTLUT:
		case G_SETTILESIZE:
		case G_LOADBLOCK:
		case G_LOADTILE:
		case G_SETTILE:
		case G_SETTIMG:
			break;
		case G_TEXRECT:
		case G_TEXRECTFLIP:
			if (_checkCoordinates)
				return sideBySide(pc);
			return true;
		default:
			return false;
		}
		pc += 8;
	}
	return false;
}

bool TexrectDrawer::addRect()
{
	DisplayWindow & wnd = dwnd();
	GraphicsDrawer &  drawer = wnd.getDrawer();
	RectVertex * pRect = drawer.m_rect;

	m_curRect = _getiRect(RDP.w0, RDP.w1);

	bool bDownUp = false;
	if (m_numRects != 0) {
		bool bContinue = false;
		if (m_otherMode == gDP.otherMode._u64 && m_mux == gDP.combine.mux) {
			if (COMPARE_COORDS(m_ulx_i, m_curRect.ulx)) {
				bContinue = COMPARE_COORDS(m_lry_i, m_curRect.uly);
				bDownUp = COMPARE_COORDS(m_uly_i, m_curRect.lry);
				bContinue |= bDownUp;
			}
			else {
				for (auto iter = m_vecRectCoords.crbegin(); iter != m_vecRectCoords.crend(); ++iter) {
					if (COMPARE_COORDS(iter->x, m_curRect.ulx) && COMPARE_COORDS(iter->y, m_curRect.uly)) {
						bContinue = true;
						break;
					}
				}
			}
		}
		if (!bContinue) {
			draw();
			drawer._updateStates(DrawingState::TexRect);
			gfxContext.enable(enable::CULL_FACE, false);
		}
	}

	if (m_numRects == 0) {
		if (!_lookAhead(true))
			return false;

		m_numRects = 1;
		m_pBuffer = frameBufferList().getCurrent();
		m_otherMode = gDP.otherMode._u64;
		m_mux = gDP.combine.mux;
		m_Z = (gDP.otherMode.depthSource == G_ZS_PRIM) ? gDP.primDepth.z : 0.0f;
		m_scissor = gDP.scissor;

		m_ulx = pRect[0].x;
		m_uly = pRect[0].y;
		m_lrx = m_max_lrx = pRect[3].x;
		m_lry = m_max_lry = pRect[3].y;

		m_ulx_i = m_curRect.ulx;
		m_uly_i = m_curRect.uly;
		m_lry_i = m_curRect.lry;

		CombinerInfo & cmbInfo = CombinerInfo::get();
		cmbInfo.update();
		cmbInfo.updateParameters();
		gfxContext.enableDepthWrite(false);
		gfxContext.enable(enable::DEPTH_TEST, false);
		gfxContext.enable(enable::BLEND, false);

		_setViewport();

		gfxContext.setScissor((s32)gDP.scissor.ulx, (s32)gDP.scissor.uly, (s32)(gDP.scissor.lrx - gDP.scissor.ulx), (s32)(gDP.scissor.lry - gDP.scissor.uly));

		gfxContext.bindFramebuffer(bufferTarget::DRAW_FRAMEBUFFER, m_FBO);
	} else {
		++m_numRects;
	}

	if (bDownUp) {
		m_ulx = pRect[0].x;
		m_uly = pRect[0].y;
		m_ulx_i = m_curRect.ulx;
		m_uly_i = m_curRect.uly;
	} else {
		m_lrx = pRect[3].x;
		m_lry = pRect[3].y;
		m_max_lrx = std::max(m_max_lrx, m_lrx);
		m_max_lry = std::max(m_max_lry, m_lry);
		m_lry_i = m_curRect.lry;
	}

	RectCoords coords;
	coords.x = m_curRect.lrx;
	coords.y = m_curRect.uly;
	m_vecRectCoords.push_back(coords);
	coords.x = m_curRect.lrx;
	coords.y = m_curRect.lry;
	m_vecRectCoords.push_back(coords);

	Context::DrawRectParameters rectParams;
	rectParams.mode = drawmode::TRIANGLE_STRIP;
	rectParams.verticesCount = 4;
	rectParams.vertices = pRect;
	rectParams.combiner = currentCombiner();
	gfxContext.drawRects(rectParams);

	if (m_numRects > 1 && !_lookAhead(false))
		draw();

	return true;
}

void TexrectDrawer::addBackgroundRect()
{
	DisplayWindow & wnd = dwnd();
	GraphicsDrawer &  drawer = wnd.getDrawer();
	RectVertex * pRect = drawer.m_rect;

	if (m_numRects == 0) {
		m_numRects = 1;
		m_pBuffer = frameBufferList().getCurrent();
		m_otherMode = gDP.otherMode._u64;
		m_mux = gDP.combine.mux;
		m_Z = (gDP.otherMode.depthSource == G_ZS_PRIM) ? gDP.primDepth.z : gSP.viewport.nearz;
		m_scissor = gDP.scissor;

		m_ulx = pRect[0].x;
		m_uly = pRect[0].y;
		m_lrx = m_max_lrx = pRect[3].x;
		m_lry = m_max_lry = pRect[3].y;

		CombinerInfo & cmbInfo = CombinerInfo::get();
		cmbInfo.update();
		cmbInfo.updateParameters();
		gfxContext.enableDepthWrite(false);
		gfxContext.enable(enable::DEPTH_TEST, false);
		gfxContext.enable(enable::BLEND, false);

		_setViewport();

		gfxContext.setScissor((s32)gDP.scissor.ulx, (s32)gDP.scissor.uly, (s32)(gDP.scissor.lrx - gDP.scissor.ulx), (s32)(gDP.scissor.lry - gDP.scissor.uly));

		gfxContext.bindFramebuffer(bufferTarget::DRAW_FRAMEBUFFER, m_FBO);
	} else {
		++m_numRects;
	}

	m_lrx = pRect[3].x;
	m_lry = pRect[3].y;
	m_max_lrx = std::max(m_max_lrx, m_lrx);
	m_max_lry = std::max(m_max_lry, m_lry);

	Context::DrawRectParameters rectParams;
	rectParams.mode = drawmode::TRIANGLE_STRIP;
	rectParams.verticesCount = 4;
	rectParams.vertices = pRect;
	rectParams.combiner = currentCombiner();
	gfxContext.drawRects(rectParams);
}

bool TexrectDrawer::draw()
{
	if (m_numRects == 0)
		return false;

	ValueKeeper<u64> otherMode(gDP.otherMode._u64, m_otherMode);
	ValueKeeper<gDPScissor> scissor(gDP.scissor, m_scissor);
	DisplayWindow & wnd = dwnd();
	GraphicsDrawer &  drawer = wnd.getDrawer();
	drawer.setBlendMode();
	gDP.changed |= CHANGED_RENDERMODE;  // Force update of depth compare parameters
	gDP.m_texCoordBounds.valid = false;
	drawer._updateDepthCompare();

	int enableAlphaTest = 0;
	switch (gDP.otherMode.cycleType) {
	case G_CYC_COPY:
		if (gDP.otherMode.alphaCompare & G_AC_THRESHOLD)
			enableAlphaTest = 1;
		break;
	case G_CYC_1CYCLE:
	case G_CYC_2CYCLE:
		if (((gDP.otherMode.alphaCompare & G_AC_THRESHOLD) != 0) && (gDP.otherMode.alphaCvgSel == 0) && (gDP.otherMode.forceBlender == 0 || gDP.blendColor.a > 0))
			enableAlphaTest = 1;
		else if ((gDP.otherMode.alphaCompare == G_AC_DITHER) && (gDP.otherMode.alphaCvgSel == 0))
			enableAlphaTest = 1;
		else if (gDP.otherMode.cvgXAlpha != 0)
			enableAlphaTest = 1;
		break;
	}

	m_lrx = m_max_lrx;
	m_lry = m_max_lry;

	RectVertex rect[4];

	f32 scaleX, scaleY;
	calcCoordsScales(m_pBuffer, scaleX, scaleY);
	scaleX *= 2.0f;
	scaleY *= 2.0f;

	const float s0 = m_ulx / (float)m_pTexture->width; // +0.5f / (float)m_pTexture->width;
	const float t0 = m_lry / (float)m_pTexture->height;// +0.5f / (float)m_pTexture->height;
	const float s1 = m_lrx / (float)m_pTexture->width;
	const float t1 = m_uly / (float)m_pTexture->height;
	const float W = 1.0f;
	const float Z = m_Z;
	constexpr float halfScreenSizeDims = SCREEN_SIZE_DIM * 0.5f;

	drawer._updateViewport(m_pBuffer);

	textureCache().activateTexture(0, m_pTexture);
	// Disable filtering to avoid black outlines
	Context::TexParameters texParams;
	texParams.handle = m_pTexture->name;
	texParams.target = textureTarget::TEXTURE_2D;
	texParams.textureUnitIndex = textureIndices::Tex[0];
	texParams.minFilter = textureParameters::FILTER_NEAREST;
	texParams.magFilter = textureParameters::FILTER_NEAREST;
	gfxContext.setTextureParameters(texParams);

	m_programTex->activate();
	m_programTex->setEnableAlphaTest(enableAlphaTest);

	float ulx = (m_ulx - halfScreenSizeDims) / halfScreenSizeDims;
	float uly = (m_uly - halfScreenSizeDims) / halfScreenSizeDims;
	float lrx = (m_lrx - halfScreenSizeDims) / halfScreenSizeDims;
	float lry = (m_lry - halfScreenSizeDims) / halfScreenSizeDims;

	rect[0].x = ulx;
	rect[0].y = lry;
	rect[0].z = Z;
	rect[0].w = W;
	rect[0].s0 = s0;
	rect[0].t0 = t0;
	rect[1].x = lrx;
	rect[1].y = lry;
	rect[1].z = Z;
	rect[1].w = W;
	rect[1].s0 = s1;
	rect[1].t0 = t0;
	rect[2].x = ulx;
	rect[2].y = uly;
	rect[2].z = Z;
	rect[2].w = W;
	rect[2].s0 = s0;
	rect[2].t0 = t1;
	rect[3].x = lrx;
	rect[3].y = uly;
	rect[3].z = Z;
	rect[3].w = W;
	rect[3].s0 = s1;
	rect[3].t0 = t1;

	drawer.updateScissor(m_pBuffer);
	_setDrawBuffer();

	Context::DrawRectParameters rectParams;
	rectParams.mode = drawmode::TRIANGLE_STRIP;
	rectParams.verticesCount = 4;
	rectParams.vertices = rect;
	rectParams.combiner = m_programTex.get();
	gfxContext.drawRects(rectParams);

	gfxContext.bindFramebuffer(bufferTarget::DRAW_FRAMEBUFFER, m_FBO);
	m_programClear->activate();

	ulx = (std::max(0.0f, m_ulx - 1.0f) - halfScreenSizeDims) / halfScreenSizeDims;
	lrx = (std::min(640.0f, m_lrx + 1.0f) - halfScreenSizeDims) / halfScreenSizeDims;
	rect[0].x = ulx;
	rect[1].x = lrx;
	rect[2].x = ulx;
	rect[3].x = lrx;

	uly = (std::max(0.0f, m_uly - 1.0f) - halfScreenSizeDims) / halfScreenSizeDims;
	lry = (std::min(580.0f, m_lry + 1.0f) - halfScreenSizeDims) / halfScreenSizeDims;
	rect[0].y = uly;
	rect[1].y = uly;
	rect[2].y = lry;
	rect[3].y = lry;

	_setViewport();

	gfxContext.enable(enable::BLEND, false);
	gfxContext.enable(enable::SCISSOR_TEST, false);
	rectParams.combiner = m_programClear.get();
	gfxContext.drawRects(rectParams);
	gfxContext.enable(enable::SCISSOR_TEST, true);

	m_pBuffer = frameBufferList().getCurrent();
	_setDrawBuffer();

	m_numRects = 0;
	m_vecRectCoords.clear();
	gDP.changed |= CHANGED_COMBINE | CHANGED_SCISSOR | CHANGED_RENDERMODE;
	gSP.changed |= CHANGED_VIEWPORT | CHANGED_TEXTURE | CHANGED_GEOMETRYMODE;

	return true;
}

bool TexrectDrawer::isEmpty() const
{
	return m_numRects == 0;
}

bool TexrectDrawer::canContinue() const
{
	return (m_numRects != 0 &&
			m_otherMode == gDP.otherMode._u64 &&
			m_mux == gDP.combine.mux &&
			m_pBuffer == frameBufferList().getCurrent());
}
