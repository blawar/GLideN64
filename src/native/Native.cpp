#include "PluginAPI.h"
#include "N64.h"
#include "RSP.h"
#include "Native.h"
#include "GLideN64.h"
#include "Config.h"
#include "DebugDump.h"
#include "Config.h"
#include "DisplayWindow.h"
#include "FrameBuffer.h"
#include "FrameBufferInfo.h"
#include <wchar.h>
#include "Settings.h"

#define START_WIDTH 1280
#define START_HEIGHT 720

static u64 g_originalWidth = START_WIDTH;//Size set by the end-user
static u64 g_width  = START_WIDTH;//Current size
static u64 g_height = START_HEIGHT;
static bool highres_hts = true;

extern "C" {
    u64 gfx_width()
    {
        return g_width;
    }

    u64 gfx_height()
    {
        return g_height;
    }
}

#ifdef WTL_UI
void ConfigInit(void* hinst);
void ConfigCleanup(void);
#endif

N64Regs::N64Regs() {
MI_INTR = new u32;
DPC_START = new word;
DPC_END = new word;
DPC_CURRENT = new word;
DPC_STATUS = new word;
DPC_CLOCK = new u32;
DPC_BUFBUSY = new u32;
DPC_PIPEBUSY = new u32;
DPC_TMEM = new u32;
VI_STATUS = new u32;
VI_ORIGIN = new u32;
VI_WIDTH = new u32;
VI_INTR = new u32;
VI_V_CURRENT_LINE = new u32;
VI_TIMING = new u32;
VI_V_SYNC = new u32;
VI_H_SYNC = new u32;
VI_LEAP = new u32;
VI_H_START = new u32;
VI_V_START = new u32;
VI_V_BURST = new u32;
VI_X_SCALE = new u32;
VI_Y_SCALE = new u32;
SP_STATUS = new u32;

*MI_INTR = 0;
*DPC_START = 0;
*DPC_END = 0;
*DPC_CURRENT = 0;
*DPC_STATUS = 0;
*DPC_CLOCK = 0;
*DPC_BUFBUSY = 0;
*DPC_PIPEBUSY = 0;
*DPC_TMEM = 0;
*VI_STATUS = 0;
*VI_ORIGIN = 0;
*VI_WIDTH = 320;
*VI_INTR = 0;
*VI_V_CURRENT_LINE = 0;
*VI_TIMING = 0;
*VI_V_SYNC = 0;
*VI_H_SYNC = 0;
*VI_LEAP = 0;
*VI_H_START = 0;
*VI_V_START = 0;
*VI_V_BURST = 0;
*VI_X_SCALE = 1024;
*VI_Y_SCALE = 512;
*SP_STATUS = 0;
}

N64Regs::~N64Regs() {
}

extern "C"
{
    //Called when the end-user changes the window size
    void gfx_resize(long width, long height)
    {
        g_originalWidth = width;

        if (config.frameBufferEmulation.aspect == 1)//Running in 4:3 mode?
            g_width = (height*4)/3;
        else
            g_width = width;
        g_height = height;

        config.video.windowedWidth  = width;
        config.video.windowedHeight = height;
        dwnd().setWindowSize(width, height);
    }
}

void _CheckInterrupts() {
}

extern "C" {
    void gfx_init(const char* romName, OSViMode* viMode) {
        REG.VI_STATUS  = &viMode->comRegs.ctrl;
        REG.VI_WIDTH   = &viMode->comRegs.width;
        REG.VI_TIMING  = &viMode->comRegs.burst;
        REG.VI_V_SYNC  = &viMode->comRegs.vSync;
        REG.VI_H_SYNC  = &viMode->comRegs.hSync;
        REG.VI_LEAP    = &viMode->comRegs.leap;
        REG.VI_H_START = &viMode->comRegs.hStart;
        REG.VI_X_SCALE = &viMode->comRegs.xScale;
        REG.VI_V_CURRENT_LINE = &viMode->comRegs.vCurrent;

        REG.VI_ORIGIN  = &viMode->fldRegs->origin;
        REG.VI_Y_SCALE = &viMode->fldRegs->yScale;
        REG.VI_V_START = &viMode->fldRegs->vStart;
        REG.VI_V_BURST = &viMode->fldRegs->vBurst;
        REG.VI_INTR    = &viMode->fldRegs->vIntr;

        CheckInterrupts = _CheckInterrupts;

        //StartDump(DEBUG_LOW | DEBUG_NORMAL | DEBUG_DETAIL | DEBUG_IGNORED | DEBUG_ERROR);

        REG.VI_STATUS;
        RDRAMSize = (word)-1;

        api().RomOpen(romName);
// Fixme: linux port (wsprintf)
#ifndef OS_LINUX
        wsprintf(config.textureFilter.txCachePath, L".");
#endif
        config.textureFilter.txHiresTextureFileStorage = highres_hts ? 1 : 0;
    }

    void gfx_switch_to_htc(bool enable) {
        highres_hts = !enable;
        config.textureFilter.txHiresTextureFileStorage = enable ? 0 : 1;
    }

    bool gfx_is_highres_enabled() {
        return config.textureFilter.txHiresEnable;
    }

    void gfx_highres_enable(bool enable) {
        config.textureFilter.txHiresEnable = enable;
    }

    void gfx_force_43(bool enable) {
        const u32 newAspectRatio = enable ? 1 : 3;

        if (config.frameBufferEmulation.aspect == newAspectRatio)
            return;//Already set

        config.frameBufferEmulation.aspect = enable ? 1 : 3;
        dwnd().forceResizeWindow();//Inform GLideN64 about the change

        //Calculate new width
        auto newWidth = g_originalWidth;
        if (enable)
            newWidth = (g_height * 4) / 3;

        g_width = newWidth;
    }

    bool gfx_force_43_enabled() {
        return config.frameBufferEmulation.aspect == 1;
    }

    void gfx_set_overscan(int left, int top, int right, int bottom) {
        config.frameBufferEmulation.enableOverscan      = 1;
        config.frameBufferEmulation.overscanPAL.left    = left;
        config.frameBufferEmulation.overscanPAL.right   = right;
        config.frameBufferEmulation.overscanPAL.top     = top;
        config.frameBufferEmulation.overscanPAL.bottom  = bottom;
        config.frameBufferEmulation.overscanNTSC.left   = left;
        config.frameBufferEmulation.overscanNTSC.right  = right;
        config.frameBufferEmulation.overscanNTSC.top    = top;
        config.frameBufferEmulation.overscanNTSC.bottom = bottom;
    }

    void gfx_shutdown() {
        RDRAMSize = 0;
        api().RomClosed();
    }

    void gfx_run(OSTask_t* task, u32 sz) {
        if(sizeof(OSTask_t) != sz)
        {
            return;
        }
        RSP_ProcessDList(task->data_ptr, task->data_size, task->ucode_boot, task->ucode_data, task->ucode_size);
        api().UpdateScreen();
        //Sleep(30);
    }

    int gfx_fbe_is_enabled() {
        return config.frameBufferEmulation.enable;
    }

    void gfx_fbe_enable(int enable) {
        config.frameBufferEmulation.enable = enable;
    }

    void gfx_fbe_sync(GraphicsContext* gfxCtx, GameInfo* GameInfo) {
        if (!config.frameBufferEmulation.enable)
            return;

        CfbInfo* cfb = gfxCtx->task.framebuffer;//Current frame buffer (according the the game)
        FrameBufferList& frameBuffers = FrameBufferList::get();//GLideN64's frame buffer list

        if (!frameBuffers.getCurrent())
            return;

        gfxCtx->curFrameBuffer = &frameBuffers.getCurrent()->m_startAddress;
        gfxCtx->viMode->fldRegs->origin = frameBuffers.getCurrent()->m_startAddress;

        cfb->fb1        = gfxCtx->curFrameBuffer;
        cfb->swapBuffer = gfxCtx->curFrameBuffer;

        cfb->viMode   = gfxCtx->viMode;
        cfb->features = gfxCtx->viFeatures;
        cfb->xScale   = gfxCtx->xScale;
        cfb->xScale   = gfxCtx->yScale;
        cfb->unk_10   = 0;
        cfb->updateRate = (s8)R_UPDATE_RATE;


        REG.VI_STATUS  = &cfb->viMode->comRegs.ctrl;
        REG.VI_WIDTH   = &cfb->viMode->comRegs.width;
        REG.VI_TIMING  = &cfb->viMode->comRegs.burst;
        REG.VI_V_SYNC  = &cfb->viMode->comRegs.vSync;
        REG.VI_H_SYNC  = &cfb->viMode->comRegs.hSync;
        REG.VI_LEAP    = &cfb->viMode->comRegs.leap;
        REG.VI_H_START = &cfb->viMode->comRegs.hStart;
        REG.VI_X_SCALE = &cfb->viMode->comRegs.xScale;
        REG.VI_V_CURRENT_LINE = &cfb->viMode->comRegs.vCurrent;

        REG.VI_ORIGIN  = &cfb->viMode->fldRegs->origin;//This is incorrect REG.VI_ORIGIN should contain only 24 bits of the frame buffer address
        REG.VI_Y_SCALE = &cfb->viMode->fldRegs->yScale;
        REG.VI_V_START = &cfb->viMode->fldRegs->vStart;
        REG.VI_V_BURST = &cfb->viMode->fldRegs->vBurst;
        REG.VI_INTR    = &cfb->viMode->fldRegs->vIntr;
    }
}

Config config;

void Config_DoConfig(/*HWND hParent*/)
{
    /*if(ConfigOpen)
        return;

    wchar_t strIniFolderPath[PLUGIN_PATH_SIZE];
    api().FindPluginPath(strIniFolderPath);

    ConfigOpen = true;
    const u32 maxMsaa = dwnd().maxMSAALevel();
    const u32 maxAnisotropy = dwnd().maxAnisotropy();
    const bool bRestart = RunConfig(strIniFolderPath, api().isRomOpen() ? RSP.romname : nullptr, maxMsaa, maxAnisotropy);
    if(config.generalEmulation.enableCustomSettings != 0)
        LoadCustomRomSettings(strIniFolderPath, RSP.romname);
    config.validate();
    if(bRestart)
        dwnd().restart();
    ConfigOpen = false;*/
}

void LoadConfig(const wchar_t* _strFileName)
{
#if defined(OS_LINUX)
    std::wstring wStrFile(_strFileName);
    std::string IniFolder(wStrFile.begin(), wStrFile.end());
    loadSettings(IniFolder.c_str());
    return;
#else
    std::string IniFolder;
    uint32_t slength = WideCharToMultiByte(CP_ACP, 0, _strFileName, -1, NULL, 0, NULL, NULL);
    IniFolder.resize(slength);
    slength = WideCharToMultiByte(CP_ACP, 0, _strFileName, -1, (LPSTR)IniFolder.c_str(), slength, NULL, NULL);
    IniFolder.resize(slength - 1); //Remove null end char

    loadSettings(IniFolder.c_str());
#endif
}

void LoadCustomRomSettings(const wchar_t* _strFileName, const char* _romName)
{
#if defined(OS_LINUX)
    std::wstring wStrFile(_strFileName);
    std::string IniFolder(wStrFile.begin(), wStrFile.end());
    loadCustomRomSettings(IniFolder.c_str(), _romName);
    return;
#else
    std::string IniFolder;
    uint32_t slength = WideCharToMultiByte(CP_ACP, 0, _strFileName, -1, NULL, 0, NULL, NULL);
    IniFolder.resize(slength);
    slength = WideCharToMultiByte(CP_ACP, 0, _strFileName, -1, (LPSTR)IniFolder.c_str(), slength, NULL, NULL);
    IniFolder.resize(slength - 1); //Remove null end char

    loadCustomRomSettings(IniFolder.c_str(), _romName);
#endif
}

void Config_LoadConfig()
{
    wchar_t strIniFolderPath[PLUGIN_PATH_SIZE];
    api().FindPluginPath(strIniFolderPath);
    LoadConfig(strIniFolderPath);
    if(config.generalEmulation.enableCustomSettings != 0)
        LoadCustomRomSettings(strIniFolderPath, RSP.romname);
    config.validate();
}
