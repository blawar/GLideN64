#include "Context.h"
#include "OpenGLContext/opengl_ContextImpl.h"

using namespace graphics;

Context gfxContext;

bool Context::Multisampling = false;
bool Context::BlitFramebuffer = false;
bool Context::WeakBlitFramebuffer = false;
bool Context::DepthFramebufferTextures = false;
bool Context::ShaderProgramBinary = false;
bool Context::ImageTextures = false;
bool Context::IntegerTextures = false;
bool Context::FramebufferFetchDepth = false;
bool Context::FramebufferFetchColor = false;
bool Context::TextureBarrier = false;
bool Context::EglImage = false;
bool Context::EglImageFramebuffer = false;
bool Context::DualSourceBlending = false;

Context::Context() {}

Context::~Context() {
	m_impl.reset();
}


void Context::init()
{
	m_impl.reset(new opengl::ContextImpl);
	m_impl->init();
	m_fbTexFormats.reset(m_impl->getFramebufferTextureFormats());
	Multisampling = m_impl->isSupported(SpecialFeatures::Multisampling);
	BlitFramebuffer = m_impl->isSupported(SpecialFeatures::BlitFramebuffer);
	WeakBlitFramebuffer = m_impl->isSupported(SpecialFeatures::WeakBlitFramebuffer);
	DepthFramebufferTextures = m_impl->isSupported(SpecialFeatures::DepthFramebufferTextures);
	ShaderProgramBinary = m_impl->isSupported(SpecialFeatures::ShaderProgramBinary);
	ImageTextures = m_impl->isSupported(SpecialFeatures::ImageTextures);
	IntegerTextures = m_impl->isSupported(SpecialFeatures::IntegerTextures);
	FramebufferFetchDepth = m_impl->isSupported(SpecialFeatures::N64DepthWithFbFetchDepth);
	FramebufferFetchColor = m_impl->isSupported(SpecialFeatures::FramebufferFetchColor);
	TextureBarrier = m_impl->isSupported(SpecialFeatures::TextureBarrier);
	EglImage = m_impl->isSupported(SpecialFeatures::EglImage);
	EglImageFramebuffer = m_impl->isSupported(SpecialFeatures::EglImageFramebuffer);
	DualSourceBlending = m_impl->isSupported(SpecialFeatures::DualSourceBlending);
}

void Context::destroy()
{
	m_impl->destroy();
	m_impl.reset();
}

void Context::setClampMode(ClampMode _mode)
{
	if (m_impl)
		m_impl->setClampMode(_mode);
}

ClampMode Context::getClampMode()
{
	return m_impl->getClampMode();
}

void Context::enable(EnableParam _parameter, bool _enable)
{
	if (m_impl)
		m_impl->enable(_parameter, _enable);
}

u32 Context::isEnabled(EnableParam _parameter)
{
	return m_impl->isEnabled(_parameter);
}

void Context::cullFace(CullModeParam _parameter)
{
	m_impl->cullFace(_parameter);
}

void Context::enableDepthWrite(bool _enable)
{
	if (m_impl)
		m_impl->enableDepthWrite(_enable);
}

void Context::setDepthCompare(CompareParam _mode)
{
	if (m_impl)
		m_impl->setDepthCompare(_mode);
}

void Context::setViewport(s32 _x, s32 _y, s32 _width, s32 _height)
{
	if (m_impl)
		m_impl->setViewport(_x, _y, _width, _height);
}

void Context::setScissor(s32 _x, s32 _y, s32 _width, s32 _height)
{
	if (m_impl)
		m_impl->setScissor(_x, _y, _width, _height);
}

void Context::setBlending(BlendParam _sfactor, BlendParam _dfactor)
{
	if (m_impl)
		m_impl->setBlending(_sfactor, _dfactor);
}

void graphics::Context::setBlendingSeparate(BlendParam _sfactorcolor, BlendParam _dfactorcolor, BlendParam _sfactoralpha, BlendParam _dfactoralpha)
{
	if (m_impl)
		m_impl->setBlendingSeparate(_sfactorcolor, _dfactorcolor, _sfactoralpha, _dfactoralpha);
}

void Context::setBlendColor(f32 _red, f32 _green, f32 _blue, f32 _alpha)
{
	if (m_impl)
		m_impl->setBlendColor(_red, _green, _blue, _alpha);
}

void Context::clearColorBuffer(f32 _red, f32 _green, f32 _blue, f32 _alpha)
{
	if (m_impl)
		m_impl->clearColorBuffer(_red, _green, _blue, _alpha);
}

void Context::clearDepthBuffer()
{
	if (m_impl)
		m_impl->clearDepthBuffer();
}

void Context::setPolygonOffset(f32 _factor, f32 _units)
{
	if (m_impl)
		m_impl->setPolygonOffset(_factor, _units);
}

ObjectHandle Context::createTexture(Parameter _target)
{
	if (!m_impl) return ObjectHandle(0);
	return m_impl->createTexture(_target);
}

void Context::deleteTexture(ObjectHandle _name)
{
	if (m_impl)
		m_impl->deleteTexture(_name);
}

void Context::init2DTexture(const InitTextureParams & _params)
{
	if (m_impl)
		m_impl->init2DTexture(_params);
}

void Context::update2DTexture(const UpdateTextureDataParams & _params)
{
	if (m_impl)
		m_impl->update2DTexture(_params);
}

void Context::setTextureParameters(const TexParameters & _parameters)
{
	if (m_impl)
		m_impl->setTextureParameters(_parameters);
}

void Context::bindTexture(const BindTextureParameters & _params)
{
	if (m_impl)
		m_impl->bindTexture(_params);
}

void Context::setTextureUnpackAlignment(s32 _param)
{
	if (m_impl)
		m_impl->setTextureUnpackAlignment(_param);
}

s32 Context::getTextureUnpackAlignment() const
{
	if (!m_impl) return 0;
	return m_impl->getTextureUnpackAlignment();
}

s32 Context::getMaxTextureSize() const
{
	if (!m_impl) return 0;
		return m_impl->getMaxTextureSize();
}

f32 Context::getMaxAnisotropy() const
{
	if (!m_impl) return 0.0f;
	return m_impl->getMaxAnisotropy();
}

void Context::bindImageTexture(const BindImageTextureParameters & _params)
{
	if (m_impl)
		m_impl->bindImageTexture(_params);
}

u32 Context::convertInternalTextureFormat(u32 _format) const
{
	if (!m_impl) return 0;
	return m_impl->convertInternalTextureFormat(_format);
}

void Context::textureBarrier()
{
	if (m_impl)
		m_impl->textureBarrier();
}

/*---------------Framebuffer-------------*/

const FramebufferTextureFormats & Context::getFramebufferTextureFormats()
{
	return *m_fbTexFormats.get();
}

ObjectHandle Context::createFramebuffer()
{
	if (!m_impl) return ObjectHandle(0);
	return m_impl->createFramebuffer();
}

void Context::deleteFramebuffer(ObjectHandle _name)
{
	if (m_impl)
		m_impl->deleteFramebuffer(_name);
}

void Context::bindFramebuffer(BufferTargetParam _target, ObjectHandle _name)
{
	if (m_impl)
		m_impl->bindFramebuffer(_target, _name);
}

ObjectHandle Context::createRenderbuffer()
{
	return m_impl->createRenderbuffer();
}

void Context::initRenderbuffer(const InitRenderbufferParams & _params)
{
	if (m_impl)
		m_impl->initRenderbuffer(_params);
}

void Context::addFrameBufferRenderTarget(const FrameBufferRenderTarget & _params)
{
	if (m_impl)
		m_impl->addFrameBufferRenderTarget(_params);
}

bool Context::blitFramebuffers(const BlitFramebuffersParams & _params)
{
	return m_impl->blitFramebuffers(_params);
}

void Context::setDrawBuffers(u32 _num)
{
	if (m_impl)
		m_impl->setDrawBuffers(_num);
}

PixelReadBuffer * Context::createPixelReadBuffer(size_t _sizeInBytes)
{
	return m_impl->createPixelReadBuffer(_sizeInBytes);
}

ColorBufferReader * Context::createColorBufferReader(CachedTexture * _pTexture)
{
	return m_impl->createColorBufferReader(_pTexture);
}

/*---------------Shaders-------------*/

bool Context::isCombinerProgramBuilderObsolete()
{
	if (!m_impl) return false;
	return m_impl->isCombinerProgramBuilderObsolete();
}

void Context::resetCombinerProgramBuilder()
{
	if (m_impl)
		m_impl->resetCombinerProgramBuilder();
}

CombinerProgram * Context::createCombinerProgram(Combiner & _color, Combiner & _alpha, const CombinerKey & _key)
{
	if (!m_impl) return nullptr;
	return m_impl->createCombinerProgram(_color, _alpha, _key);
}

bool Context::saveShadersStorage(const Combiners & _combiners)
{
	if (!m_impl) return false;
	return m_impl->saveShadersStorage(_combiners);
}

bool Context::loadShadersStorage(Combiners & _combiners)
{
	if (!m_impl) return false;
	return m_impl->loadShadersStorage(_combiners);
}

ShaderProgram * Context::createDepthFogShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createDepthFogShader();
}

TexrectDrawerShaderProgram * Context::createTexrectDrawerDrawShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTexrectDrawerDrawShader();
}

ShaderProgram * Context::createTexrectDrawerClearShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTexrectDrawerClearShader();
}

ShaderProgram * Context::createTexrectUpscaleCopyShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTexrectUpscaleCopyShader();
}

ShaderProgram * Context::createTexrectColorAndDepthUpscaleCopyShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTexrectColorAndDepthUpscaleCopyShader();
}

ShaderProgram * Context::createTexrectDownscaleCopyShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTexrectDownscaleCopyShader();
}

ShaderProgram * Context::createTexrectColorAndDepthDownscaleCopyShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTexrectColorAndDepthDownscaleCopyShader();
}

ShaderProgram * Context::createGammaCorrectionShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createGammaCorrectionShader();
}

ShaderProgram * Context::createFXAAShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createFXAAShader();
}

TextDrawerShaderProgram * Context::createTextDrawerShader()
{
	if (!m_impl) return nullptr;
	return m_impl->createTextDrawerShader();
}

void Context::resetShaderProgram()
{
	if (m_impl)
		m_impl->resetShaderProgram();
}

void Context::drawTriangles(const DrawTriangleParameters & _params)
{
	if (m_impl)
		m_impl->drawTriangles(_params);
}

void Context::drawRects(const DrawRectParameters & _params)
{
	if (m_impl)
		m_impl->drawRects(_params);
}

void Context::drawLine(f32 _width, SPVertex * _vertices)
{
	m_impl->drawLine(_width, _vertices);
}

f32 Context::getMaxLineWidth()
{
	if (!m_impl) return 0.0f;
	return m_impl->getMaxLineWidth();
}

s32 Context::getMaxMSAALevel()
{
	if (!m_impl) return 0;
	return m_impl->getMaxMSAALevel();
}

bool Context::isError() const
{
	if (!m_impl) return false;
	return m_impl->isError();
}

bool Context::isFramebufferError() const
{
	if (!m_impl) return false;
	return m_impl->isFramebufferError();
}
