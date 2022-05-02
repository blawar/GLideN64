glidenhq_sources = ['TextureFilters.cpp', 'TextureFilters_2xsai.cpp', 'TextureFilters_hq2x.cpp', 'TextureFilters_hq4x.cpp', 'TextureFilters_xbrz.cpp', 'TxCache.cpp', 'TxDbg.cpp', 'TxFilter.cpp', 'TxFilterExport.cpp', 'TxHiResLoader.cpp', 'TxHiResCache.cpp', 'TxHiResNoCache.cpp', 'TxImage.cpp', 'TxQuantize.cpp', 'TxReSample.cpp', 'TxTexCache.cpp', 'TxUtil.cpp']

pic_flags = []
glidenhq_includes = include_directories('.')

# platform specific files
message('target_os = ' + target_os)
if target_os == 'linux'
  gliden64_sources += gliden64_sources_unix
  args_platform += [ '-DOS_LINUX', '-DNDEBUG']
  pic_flags += '-fPIC'
elif target_os == 'darwin'
  gliden64_sources += gliden64_sources_unix
  args_platform += [ '-DOS_MAC_OS_X', '-DNDEBUG']
  pic_flags += '-fPIC'
elif target_os == 'windows'
  gliden64_sources += gliden64_sources_win
  args_platform += [ '-DWIN32', '-DOS_WINDOWS', '-D_CRT_SECURE_NO_WARNINGS']
  arg_linker += [ '-lopengl32' ]
#if '${CMAKE_CXX_COMPILER_ID}', 'STREQUAL', 'MSVC'
#  # add_definitions('-D__MSC__')
#endif
endif

# Build type
glidenhq_name = 'GLideNHQ'
if get_option('buildtype') == 'debug'
  glidenhq_name += 'd'
endif

glidenhq_lib = static_library('GLideNHQd', glidenhq_sources,
  include_directories : [ osal_includes, gliden64_includes, glidenhq_includes ],
  cpp_args : [ args_platform ] )

libpng_dep = dependency('libpng', fallback : ['libpng', 'libpng_dep'], native: false, default_options: ['default_library=static'])
zlib_dep = dependency('zlib', fallback : ['zlib', 'zlib_dep'], native: false, default_options: ['default_library=static'])

glidenhq_dep = declare_dependency(include_directories : '.', link_with : glidenhq_lib, dependencies : [libpng_dep, zlib_dep, libosal_dep])
