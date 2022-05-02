#include <GLideN64.h>
#include <Config.h>
#include <N64.h>
#include <RSP.h>
#include <FrameBuffer.h>
#include <Graphics/Context.h>
#include <Graphics/Parameters.h>
#include <DisplayWindow.h>
#include <windows/ScreenShot.h>
#include <Graphics/OpenGLContext/ThreadedOpenGl/opengl_Wrapper.h>


bool isMemoryWritable(void * ptr, size_t byteCount) {
	return true;
}

using namespace opengl;

class DisplayWindowWindows : public DisplayWindow
{
public:
	DisplayWindowWindows() = default;

private:
	bool _start() override;
	void _stop() override;
	void _restart() override;
	void _swapBuffers() override;
	void _saveScreenshot() override;
	void _saveBufferContent(graphics::ObjectHandle _fbo, CachedTexture *_pTexture) override;
	bool _resizeWindow() override;
	void _changeWindow() override;
	void _readScreen(void **_pDest, long *_pWidth, long *_pHeight) override;
	void _readScreen2(void * _dest, int * _width, int * _height, int _front) override {}
	graphics::ObjectHandle _getDefaultFramebuffer() override;
};

DisplayWindow & DisplayWindow::get()
{
	static DisplayWindowWindows video;
	return video;
}

bool DisplayWindowWindows::_start()
{
	FunctionWrapper::setThreadedMode(config.video.threadedVideo);

	FunctionWrapper::windowsStart();
	return _resizeWindow();
}

void DisplayWindowWindows::_stop()
{
	FunctionWrapper::windowsStop();
}

void DisplayWindowWindows::_restart()
{

}

void DisplayWindowWindows::_swapBuffers()
{
	//Don't let the command queue grow too big buy waiting on no more swap buffers being queued
	FunctionWrapper::WaitForSwapBuffersQueued();

	FunctionWrapper::windowsSwapBuffers();
}

void DisplayWindowWindows::_saveScreenshot()
{
	unsigned char * pixelData = NULL;
	GLint oldMode;
	glGetIntegerv(GL_READ_BUFFER, &oldMode);
	gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, graphics::ObjectHandle::defaultFramebuffer);
	glReadBuffer(GL_FRONT);
	pixelData = (unsigned char*)malloc(m_screenWidth * m_screenHeight * 3);
	glReadPixels(0, m_heightOffset, m_screenWidth, m_screenHeight, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	if (graphics::BufferAttachmentParam(oldMode) == graphics::bufferAttachment::COLOR_ATTACHMENT0) {
		FrameBuffer * pBuffer = frameBufferList().getCurrent();
		if (pBuffer != nullptr)
			gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, pBuffer->m_FBO);
	}
	glReadBuffer(oldMode);

	size_t size = (wcslen(m_strScreenDirectory) + 1) * sizeof(wchar_t);
	char *cBuf = new char[size];
	std::wcstombs(cBuf, m_strScreenDirectory, size);

	SaveScreenshot(cBuf, RSP.romname, m_screenWidth, m_screenHeight, pixelData);
	delete cBuf;
	free( pixelData );
}

void DisplayWindowWindows::_saveBufferContent(graphics::ObjectHandle _fbo, CachedTexture *_pTexture)
{
	unsigned char * pixelData = NULL;
	GLint oldMode;
	glGetIntegerv(GL_READ_BUFFER, &oldMode);
	gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, _fbo);
	pixelData = (unsigned char*)malloc(_pTexture->width * _pTexture->height * 3);
	glReadPixels(0, 0, _pTexture->width, _pTexture->height, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	if (graphics::BufferAttachmentParam(oldMode) == graphics::bufferAttachment::COLOR_ATTACHMENT0) {
		FrameBuffer * pCurrentBuffer = frameBufferList().getCurrent();
		if (pCurrentBuffer != nullptr)
			gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, pCurrentBuffer->m_FBO);
	}
	glReadBuffer(oldMode);

	size_t size = (wcslen(m_strScreenDirectory) + 1) * sizeof(wchar_t);
	char *cBuf = new char[size];
	std::wcstombs(cBuf, m_strScreenDirectory, size);

	SaveScreenshot(cBuf, RSP.romname, _pTexture->width, _pTexture->height, pixelData);
	delete cBuf;
	free(pixelData);
}

void DisplayWindowWindows::_changeWindow()
{
	_resizeWindow();
}

bool DisplayWindowWindows::_resizeWindow()
{
	m_screenWidth = m_width = config.video.windowedWidth;
	m_screenHeight = config.video.windowedHeight;
	_setBufferSize();
	return true;
}

void DisplayWindowWindows::_readScreen(void **_pDest, long *_pWidth, long *_pHeight)
{
	*_pWidth = m_width;
	*_pHeight = m_height;

	*_pDest = malloc(m_height * m_width * 3);
	if (*_pDest == nullptr)
		return;

#ifndef GLESX
	GLint oldMode;
	glGetIntegerv(GL_READ_BUFFER, &oldMode);
	gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, graphics::ObjectHandle::defaultFramebuffer);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, m_heightOffset, m_width, m_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, *_pDest);
	if (graphics::BufferAttachmentParam(oldMode) == graphics::bufferAttachment::COLOR_ATTACHMENT0) {
		FrameBuffer * pBuffer = frameBufferList().getCurrent();
		if (pBuffer != nullptr)
			gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, pBuffer->m_FBO);
	}
	glReadBuffer(oldMode);
#else
	glReadPixels(0, m_heightOffset, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, *_pDest);
#endif
}

graphics::ObjectHandle DisplayWindowWindows::_getDefaultFramebuffer()
{
	return graphics::ObjectHandle::null;
}
