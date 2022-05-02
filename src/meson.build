# Platform config
target_os = target_machine.system()
build_os = build_machine.system()

# 32 bit
arch_bits = '-m32'
if get_option('x64')
  arch_bits = []
endif
if not meson.is_subproject()
  add_global_arguments(arch_bits, language : ['c', 'cpp'])
endif
arg_linker = [ arch_bits ]
args_platform = []
arg_options = ['-DGBI_FLOATS']
gliden64_sources_platform = []

opengl_dep = dependency('gl')
thread_dep = dependency('threads', native: false)
freetype2_dep = dependency('freetype2', native: false, default_options: ['default_library=static', 'png=disabled', 'zlib=disabled'])
sdl2_dep = dependency('sdl2', native: false)
libpng_dep = dependency('libpng', fallback : ['libpng', 'libpng_dep'], native: false, default_options: ['default_library=static'])
zlib_dep = dependency('zlib', fallback : ['zlib', 'zlib_dep'], native: false, default_options: ['default_library=static', 'solo=true'])
gliden64_deps = [ sdl2_dep, freetype2_dep, libpng_dep, zlib_dep, thread_dep, opengl_dep ]
args_deps = ['-DUSE_SDL', '-DUSE_FREETYPE']

if build_os == 'linux' or build_os == 'darwin'
  script_extension = '.sh'
elif build_os == 'windows'
  script_extension = '.bat'
endif

# run script to generate Revision.h
# check if system has working git and we are in a git repo
fs = import('fs')
get_revision = find_program( [ 'getRevision', 'getRevision'+ script_extension ], required : false)
git = find_program('git', required : false)
if git.found() and ( fs.is_dir(meson.source_root() + '/../.git') or fs.is_file(meson.source_root() + '/../.git') )
  revision_h = custom_target(
      'revision.h',
      output : 'revision.h',
      command : [ get_revision ] )
else
  revision_h = custom_target(
      'revision.h',
      output : 'revision.h',
      command : [ get_revision, '--nogit' ] )
endif

gliden64_sources = [
  '3DMath.cpp', 
  'Combiner.cpp', 
  'CombinerKey.cpp', 
  'CommonPluginAPI.cpp', 
  'Config.cpp', 
  'convert.cpp', 
  'DebugDump.cpp', 
  'Debugger.cpp', 
  'DepthBuffer.cpp', 
  'DisplayWindow.cpp', 
  'DisplayLoadProgress.cpp', 
  'FrameBuffer.cpp', 
  'FrameBufferInfo.cpp', 
  'GBI.cpp', 
  'gDP.cpp', 
  'GLideN64.cpp', 
  'GraphicsDrawer.cpp', 
  'gSP.cpp', 
  'Log.cpp', 
  'N64.cpp', 
  'NoiseTexture.cpp', 
  'PaletteTexture.cpp', 
  'Performance.cpp', 
  'PostProcessor.cpp', 
  'RDP.cpp', 
  'RSP.cpp', 
  'RSP_LoadMatrix.cpp', 
  'SoftwareRender.cpp', 
  'TexrectDrawer.cpp', 
  'TextDrawer.cpp', 
  'TextureFilterHandler.cpp', 
  'Textures.cpp', 
  'VI.cpp', 
  'ZlutTexture.cpp', 
  'BufferCopy/BlueNoiseTexture.cpp', 
  'BufferCopy/ColorBufferToRDRAM.cpp', 
  'BufferCopy/DepthBufferToRDRAM.cpp', 
  'BufferCopy/RDRAMtoColorBuffer.cpp', 
  'DepthBufferRender/ClipPolygon.cpp', 
  'DepthBufferRender/DepthBufferRender.cpp', 
  'common/CommonAPIImpl_common.cpp', 
  'native/FileClass.cpp', 
  'native/GlSettings.cpp', 
  'native/IniFileClass.cpp', 
  'native/Native.cpp', 
  'native/NativeSettings.cpp', 
  'native/util/util.cpp', 
  'native/util/path.cpp', 
  'native/util/StdString.cpp', 
  'Graphics/Context.cpp', 
  'Graphics/ColorBufferReader.cpp', 
  'Graphics/CombinerProgram.cpp', 
  'Graphics/ObjectHandle.cpp', 
  'Graphics/OpenGLContext/GLFunctions.cpp', 
  'Graphics/OpenGLContext/ThreadedOpenGl/opengl_Command.cpp', 
  'Graphics/OpenGLContext/ThreadedOpenGl/opengl_ObjectPool.cpp', 
  'Graphics/OpenGLContext/ThreadedOpenGl/opengl_Wrapper.cpp', 
  'Graphics/OpenGLContext/ThreadedOpenGl/opengl_WrappedFunctions.cpp', 
  'Graphics/OpenGLContext/ThreadedOpenGl/RingBufferPool.cpp', 
  'Graphics/OpenGLContext/opengl_Attributes.cpp', 
  'Graphics/OpenGLContext/opengl_BufferedDrawer.cpp', 
  'Graphics/OpenGLContext/opengl_BufferManipulationObjectFactory.cpp', 
  'Graphics/OpenGLContext/opengl_CachedFunctions.cpp', 
  'Graphics/OpenGLContext/opengl_ColorBufferReaderWithBufferStorage.cpp', 
  'Graphics/OpenGLContext/opengl_ColorBufferReaderWithPixelBuffer.cpp', 
  'Graphics/OpenGLContext/opengl_ColorBufferReaderWithReadPixels.cpp', 
  'Graphics/OpenGLContext/opengl_ContextImpl.cpp', 
  'Graphics/OpenGLContext/opengl_GLInfo.cpp', 
  'Graphics/OpenGLContext/opengl_Parameters.cpp', 
  'Graphics/OpenGLContext/opengl_TextureManipulationObjectFactory.cpp', 
  'Graphics/OpenGLContext/opengl_UnbufferedDrawer.cpp', 
  'Graphics/OpenGLContext/opengl_Utils.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerInputs.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramBuilder.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramBuilderCommon.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramBuilderAccurate.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramBuilderFast.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramImpl.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramUniformFactory.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramUniformFactoryCommon.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramUniformFactoryAccurate.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_CombinerProgramUniformFactoryFast.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_FXAA.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_ShaderStorage.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_SpecialShadersFactory.cpp', 
  'Graphics/OpenGLContext/GLSL/glsl_Utils.cpp', 
  'uCodes/F3D.cpp', 
  'uCodes/F3DBETA.cpp', 
  'uCodes/F3DDKR.cpp', 
  'uCodes/F3DEX.cpp', 
  'uCodes/F3DAM.cpp', 
  'uCodes/F3DEX2.cpp', 
  'uCodes/F3DEX2ACCLAIM.cpp', 
  'uCodes/F3DEX2CBFD.cpp', 
  'uCodes/F3DZEX2.cpp', 
  'uCodes/F3DFLX2.cpp', 
  'uCodes/F3DGOLDEN.cpp', 
  'uCodes/F3DTEXA.cpp', 
  'uCodes/F3DPD.cpp', 
  'uCodes/F3DSETA.cpp', 
  'uCodes/F5Indi_Naboo.cpp', 
  'uCodes/F5Rogue.cpp', 
  'uCodes/L3D.cpp', 
  'uCodes/L3DEX2.cpp', 
  'uCodes/L3DEX.cpp', 
  'uCodes/S2DEX2.cpp', 
  'uCodes/S2DEX.cpp', 
  'uCodes/T3DUX.cpp', 
  'uCodes/Turbo3D.cpp', 
  'uCodes/ZSort.cpp', 
  'uCodes/ZSortBOSS.cpp']
  
#glidenui_sources = ['ZilmarPluginAPI.cpp', 'windows/ZilmarAPIImpl_windows.cpp', 'GLideNUI/Config_GLideNUI.cpp', 'GLideNUI/AboutDialog.cpp', 'GLideNUI/AboutDialog.ui', 'GLideNUI/ConfigDialog.cpp', 'GLideNUI/Config_GLideNUI.cpp', 'GLideNUI/GLideNUI.cpp', 'GLideNUI/Settings.cpp', 'GLideNUI/QtKeyToHID.cpp', 'GLideNUI/HIDKeyToName.cpp', 'GLideNUI/configDialog.ui', 'GLideNUI/icon.qrc']

# Build type
gliden64_dll_name = 'GLideN64'
if (get_option('NO_ASSERT')) or (get_option('buildtype') == 'release')
  args_platform += [ '-UDEBUG', '-DNDEBUG' ]
  message('Building Release version of GLideN64')
else
  gliden64_dll_name = 'GLideN64d'
  args_platform += [ '-UNDEBUG', '-DDEBUG' ]
  message('Building Debug version of GLideN64')
endif

# Compiler detection
cc = meson.get_compiler('c')
args_sdl = []
sdl_include_path_full = '''#include <SDL2/SDL.h>
void func() { SDL_Init(0); }
'''
sdl_include_path_short = '''#include <SDL.h>
void func() { SDL_Init(0); }
'''
if sdl2_dep.type_name() == 'internal'
  args_sdl += [ '-DUSE_SDL2_INCLUDE_PATH_SHORT' ]
  message('Using SDL2 include path  <SDL.h>')
else
  result_sdl_path_long = cc.compiles( sdl_include_path_full, name : 'sdl_include_long', dependencies : sdl2_dep )
  if result_sdl_path_long
    message('Using SDL2 include path <SDL2/SDL.h>')
  else
    result_sdl_path_short = cc.compiles( sdl_include_path_short, name : 'sdl_include_short', dependencies : sdl2_dep )
    if result_sdl_path_short
      args_sdl += [ '-DUSE_SDL2_INCLUDE_PATH_SHORT' ]
      message('Using SDL2 include path  <SDL.h>')
    else
      error('Could not find SDL2 include path')
    endif
  endif
endif
# Add what we learned to the platform args
args_platform += args_sdl

gliden64_sources_unix = ['Graphics/OpenGLContext/linux/windows_DisplayWindow.cpp', 'Graphics/OpenGLContext/linux/WindowsWGL.cpp', 'linux/CommonAPIImpl_linux.cpp', 'windows/ScreenShot.cpp']
gliden64_sources_win = [ 'windows/CommonAPIImpl_windows.cpp', 'windows/MemoryStatus_windows.cpp',  'windows/ScreenShot.cpp', 'Graphics/OpenGLContext/windows/windows_DisplayWindow.cpp', 'Graphics/OpenGLContext/windows/WindowsWGL.cpp', 'windows/GLideN64_windows.cpp']
gliden64_includes = include_directories('.')

# Subdir projects
subdir('osal')

gliden64_deps += [ libosal_dep ]

# platform specific files
message('target_os = ' + target_os)
if target_os == 'linux' or target_os == 'darwin'
  gliden64_sources += gliden64_sources_unix
  args_platform += [ '-DOS_LINUX', '-DNATIVE', '-DGL_USE_UNIFORMBLOCK' ]
  arg_linker += [ '-pthread' ]
  dl_dep = cc.find_library('dl')
  gliden64_deps += dl_dep
elif target_os == 'windows'
  gliden64_sources += gliden64_sources_win
  args_platform += [ '-DOS_WINDOWS', '-D__WIN32__', '-DWIN32', '-D_WIN32_ASM', '-D_CRT_SECURE_NO_WARNINGS', '-D__MSC__', '-DNATIVE', '-DGL_USE_UNIFORMBLOCK']
  #arg_linker += [ '-lopengl32' ]
endif
#if 'PANDORA'
#  #Pandora as a SGX530, but it should share the bugs and limitations as SGX540
#  args_platform += [ '-DPANDORA', '-DPowerVR_SGX_540', '-DEGL' ]
#endif
#if 'ODROID'
#  args_platform += [ '-DODROID' ]
#endif
#if 'VERO4K'
#  args_platform += [ '-DVERO4K' ]
#endif

if get_option('VEC4_OPT')
  arg_options += [ '-D__VEC4_OPT' ]
endif
if get_option('CRC_ARMV8')
  gliden64_sources_platform += [ 'CRC32_ARMV8.cpp' ]
elif get_option('CRC_OPT')
  gliden64_sources_platform += [ 'CRC_OPT.cpp' ]
elif get_option('CRC_NEON')
  gliden64_sources_platform += [ 'Neon/CRC_OPT_NEON.cpp' ]
else
  gliden64_sources_platform += [ 'CRC32.cpp' ]
endif
if get_option('X86_OPT')
  gliden64_sources_platform += [ '3DMath.cpp', 'RSP_LoadMatrixX86.cpp' ]
elif get_option('NEON_OPT')
  gliden64_sources_platform += [ 'Neon/3DMathNeon.cpp', 'Neon/gSPNeon.cpp', 'Neon/RSP_LoadMatrixNeon.cpp' ]
else
  gliden64_sources_platform += [ '3DMath.cpp', 'RSP_LoadMatrix.cpp' ]
endif

if get_option('NOHQ')
  gliden64_sources_platform += [ 'TxFilterStub.cpp' ]
else
  message('Compiling GLideN64 with GLideNHQ support!')
  subdir('GLideNHQ')
  gliden64_deps += [ glidenhq_dep ]
endif

if get_option('GL_PROFILE')
  arg_options += [ '-DGL_PROFILE', '-DGL_DEBUG' ]
endif

#if 'EGL'
#  # add_definitions('-DEGL')
#  opengl_libraries = egl_lib
#endif

glidne64_dll = shared_library(gliden64_dll_name, sources: [ gliden64_sources, gliden64_sources_platform, revision_h ] , include_directories : gliden64_includes,
  cpp_args : [ arg_options, args_platform] , link_args : arg_linker,
  dependencies : gliden64_deps,
  name_prefix: '',
  install : true)

gliden64_dep = declare_dependency( link_args : arg_linker, link_with : glidne64_dll )