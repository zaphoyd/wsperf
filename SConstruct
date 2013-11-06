import os, sys, commands
env = Environment(ENV = os.environ)

# figure out a better way to configure this
if os.environ.has_key('CXX'):
   env['CXX'] = os.environ['CXX']

if os.environ.has_key('DEBUG'):
   env['DEBUG'] = os.environ['DEBUG']

if os.environ.has_key('CXXFLAGS'):
   env.Append(CXXFLAGS = os.environ['CXXFLAGS'])

if os.environ.has_key('LINKFLAGS'):
   env.Append(LINKFLAGS = os.environ['LINKFLAGS'])


## WebSocket++
##
if not os.environ.has_key('WSPP_ROOT'):
   raise SCons.Errors.UserError, "WebSocket++ directory not set: missing WSPP_ROOT environment variable"


## Boost
##
## Note: You need to either set BOOSTROOT to the root of a stock Boost distribution
## or set BOOST_INCLUDES and BOOST_LIBS if Boost comes with your OS distro e.g. and
## needs BOOST_INCLUDES=/usr/include/boost and BOOST_LIBS=/usr/lib like Ubuntu.
##
if os.environ.has_key('BOOSTROOT'):
   os.environ['BOOST_ROOT'] = os.environ['BOOSTROOT']

if os.environ.has_key('BOOST_ROOT'):
   env['BOOST_INCLUDES'] = os.path.join(os.environ['BOOST_ROOT'], 'include')
   env['BOOST_LIBS'] = os.path.join(os.environ['BOOST_ROOT'], 'lib')
elif os.environ.has_key('BOOST_INCLUDES') and os.environ.has_key('BOOST_LIBS'):
   env['BOOST_INCLUDES'] = os.environ['BOOST_INCLUDES']
   env['BOOST_LIBS'] = os.environ['BOOST_LIBS']
else:
   raise SCons.Errors.UserError, "Neither BOOST_ROOT, nor BOOST_INCLUDES + BOOST_LIBS was set!"

env.Append(CPPPATH = [env['BOOST_INCLUDES']])
env.Append(LIBPATH = [env['BOOST_LIBS']])

if os.environ.has_key('WSPP_ENABLE_CPP11'):
   env['WSPP_ENABLE_CPP11'] = True
else:
   env['WSPP_ENABLE_CPP11'] = False

boost_linkshared = False

def boostlibs(libnames,localenv):
   if localenv['PLATFORM'].startswith('win'):
      # Win/VC++ supports autolinking. nothing to do.
      # http://www.boost.org/doc/libs/1_49_0/more/getting_started/windows.html#auto-linking
      return []
   else:
      libs = []
      prefix = localenv['SHLIBPREFIX'] if boost_linkshared else localenv['LIBPREFIX']
      suffix = localenv['SHLIBSUFFIX'] if boost_linkshared else localenv['LIBSUFFIX']
      for name in libnames:
         lib = File(os.path.join(localenv['BOOST_LIBS'], '%sboost_%s%s' % (prefix, name, suffix)))
         libs.append(lib)
      return libs

if env['PLATFORM'].startswith('win'):
   env.Append(CPPDEFINES = ['WIN32',
                            'NDEBUG',
                            'WIN32_LEAN_AND_MEAN',
                            '_WIN32_WINNT=0x0600',
                            '_CONSOLE',
                            'BOOST_TEST_DYN_LINK',
                            'NOMINMAX',
                            '_WEBSOCKETPP_CPP11_MEMORY_',
                            '_WEBSOCKETPP_CPP11_FUNCTIONAL_'])
   arch_flags  = '/arch:SSE2'
   opt_flags   = '/Ox /Oi /fp:fast'
   warn_flags  = '/W3 /wd4996 /wd4995 /wd4355'
   env['CCFLAGS'] = '%s /EHsc /GR /GS- /MD /nologo %s %s' % (warn_flags, arch_flags, opt_flags)
   env['LINKFLAGS'] = '/INCREMENTAL:NO /MANIFEST /NOLOGO /OPT:REF /OPT:ICF /MACHINE:X86'
elif env['PLATFORM'] == 'posix':
   if env.has_key('DEBUG'):
      env.Append(CCFLAGS = ['-g', '-O0'])
   else:
      env.Append(CPPDEFINES = ['NDEBUG'])
      env.Append(CCFLAGS = ['-O3'])
      #env.Append(CCFLAGS = ['-O3', '-fomit-frame-pointer'])
   env.Append(CCFLAGS = ['-Wall'])
   #env['LINKFLAGS'] = ''
elif env['PLATFORM'] == 'darwin':
   if env.has_key('DEBUG'):
      env.Append(CCFLAGS = ['-g', '-O0'])
   else:
      env.Append(CPPDEFINES = ['NDEBUG'])
      env.Append(CCFLAGS = ['-O3'])
      #env.Append(CCFLAGS = ['-O3', '-fomit-frame-pointer'])
   env.Append(CCFLAGS = ['-Wall'])
   #env['LINKFLAGS'] = ''

if env['PLATFORM'].startswith('win'):
   #env['LIBPATH'] = env['BOOST_LIBS']
   pass
else:
   env['LIBPATH'] = ['/usr/lib',
                     '/usr/local/lib'] #, env['BOOST_LIBS']

# Compiler specific warning flags
if env['CXX'].startswith('g++'):
   #env.Append(CCFLAGS = ['-Wconversion'])
   env.Append(CCFLAGS = ['-Wcast-align'])
elif env['CXX'].startswith('clang++'):
   #env.Append(CCFLAGS = ['-Wcast-align'])
   #env.Append(CCFLAGS = ['-Wglobal-constructors'])
   #env.Append(CCFLAGS = ['-Wconversion'])
   env.Append(CCFLAGS = ['-Wno-padded'])

   # Wpadded
   # Wsign-conversion

platform_libs = []
tls_libs = []

tls_build = False

if env['PLATFORM'] == 'posix':
   platform_libs = ['pthread', 'rt']
   tls_libs = ['ssl', 'crypto']
   tls_build = True
elif env['PLATFORM'] == 'darwin':
   tls_libs = ['ssl', 'crypto']
   tls_build = True
elif env['PLATFORM'].startswith('win'):
   # Win/VC++ supports autolinking. nothing to do.
   pass


##### Set up C++11 environment
polyfill_libs = [] # boost libraries used as drop in replacements for incomplete
				   # C++11 STL implementations
env_cpp11 = env.Clone ()

if env_cpp11['CXX'].startswith('g++'):
   # TODO: check g++ version
   GCC_VERSION = commands.getoutput(env_cpp11['CXX'] + ' -dumpversion')

   if GCC_VERSION > "4.4.0":
      print "C++11 build environment partially enabled"
      env_cpp11.Append(WSPP_CPP11_ENABLED = "true",CXXFLAGS = ['-std=c++0x'],TOOLSET = ['g++'],CPPDEFINES = ['_WEBSOCKETPP_CPP11_STL_'])
   else:
      print "C++11 build environment is not supported on this version of G++"
elif env_cpp11['CXX'].startswith('clang++'):
   print "C++11 build environment enabled"
   env_cpp11.Append(WSPP_CPP11_ENABLED = "true",CXXFLAGS = ['-std=c++0x','-stdlib=libc++'],LINKFLAGS = ['-stdlib=libc++'],TOOLSET = ['clang++'],CPPDEFINES = ['_WEBSOCKETPP_CPP11_STL_'])

   # look for optional second boostroot compiled with clang's libc++ STL library
   # this prevents warnings/errors when linking code built with two different
   # incompatible STL libraries.
   if os.environ.has_key('BOOST_ROOT_CPP11'):
      env_cpp11['BOOST_INCLUDES'] = os.environ['BOOST_ROOT_CPP11']
      env_cpp11['BOOST_LIBS'] = os.path.join(os.environ['BOOST_ROOT_CPP11'], 'stage', 'lib')
   elif os.environ.has_key('BOOST_INCLUDES_CPP11') and os.environ.has_key('BOOST_LIBS_CPP11'):
      env_cpp11['BOOST_INCLUDES'] = os.environ['BOOST_INCLUDES_CPP11']
      env_cpp11['BOOST_LIBS'] = os.environ['BOOST_LIBS_CPP11']
else:
   print "C++11 build environment disabled"

# if the build system is known to allow the isystem modifier for library include
# values then use it for the boost libraries. Otherwise just add them to the
# regular CPPPATH values.
if env['CXX'].startswith('g++') or env['CXX'].startswith('clang'):
	env.Append(CPPFLAGS = '-isystem ' + env['BOOST_INCLUDES'])
else:
	env.Append(CPPPATH = [env['BOOST_INCLUDES']])
env.Append(LIBPATH = [env['BOOST_LIBS']])

# if the build system is known to allow the isystem modifier for library include
# values then use it for the boost libraries. Otherwise just add them to the
# regular CPPPATH values.
if env_cpp11['CXX'].startswith('g++') or env_cpp11['CXX'].startswith('clang'):
	env_cpp11.Append(CPPFLAGS = '-isystem ' + env_cpp11['BOOST_INCLUDES'])
else:
	env_cpp11.Append(CPPPATH = [env_cpp11['BOOST_INCLUDES']])
env_cpp11.Append(LIBPATH = [env_cpp11['BOOST_LIBS']])


env.Append(CPPPATH = [os.environ['WSPP_ROOT']])
env_cpp11.Append(CPPPATH = [os.environ['WSPP_ROOT']])

## END OF CONFIG !!

## TARGETS:

env = env.Clone ()
env_cpp11 = env_cpp11.Clone ()

## strip executable (this remove symbol tables - but we may want it even for
## non-debug builds for Linux "perf")
##
STRIP = False

# if a C++11 environment is available build using that, otherwise use boost
if env_cpp11.has_key('WSPP_CPP11_ENABLED'):

   ALL_LIBS = boostlibs(['system'],env_cpp11) + [platform_libs] + [polyfill_libs] + [tls_libs]
   wsperf = env_cpp11.Program('wsperf', ["wsperf.cpp"], LIBS = ALL_LIBS)

else:

   ALL_LIBS = boostlibs(['system'],env) + [platform_libs] + [polyfill_libs] + [tls_libs]
   wsperf = env.Program('wsperf', ["wsperf.cpp"], LIBS = ALL_LIBS)


if not env.has_key('DEBUG') and STRIP:
   env.AddPostAction (wsperf, env.Action('strip ' + str(wsperf[0])))
