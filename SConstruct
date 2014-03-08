# The binary crashes when executed in browser.
# I suspect .cpp -> .o -> .bc -> .pexe conversion is wrong (make version is .cpp -> .bc -> .pexe)

# default header
import os

NACL_SDK_ROOT = "/home/xyx/nacl_sdk/pepper_33"

# Setup PNaCl tools
PNACL_TOOLCHAIN_PATH = NACL_SDK_ROOT + "/toolchain/linux_pnacl"

# generated command works when manually executed in terminal,
# but when in scons it emits lots of "undefined reference to..." errors
def generate_build_bitcode(target, source, env, for_signature):
	options = []
	options += env['CXXFLAGS'].split() if type(env['CXXFLAGS']) == str else env['CXXFLAGS']
	options += env['CCFLAGS'].split() if type(env['CCFLAGS']) == str else env['CCFLAGS']
	options += ['-I' + path for path in env['CPPPATH']]
	options += ['-L' + path for path in env['LIBPATH']]
	options += ['-l' + lib for lib in env['LIBS']]
	return '%s -o %s %s %s' % (env['CXX'], target[0], source[0], ' '.join(options))


env = Environment(
	CXX = PNACL_TOOLCHAIN_PATH + "/bin/pnacl-clang++",
	CXXFLAGS = '-std=gnu++11 -pthread -Wno-long-long -Wall -Wswitch-enum -pedantic',
	CCFLAGS = ['-O2'],
	CPPPATH = [
		NACL_SDK_ROOT + "/include",
		PNACL_TOOLCHAIN_PATH + "/usr/include",
	],
	LIBPATH = [
		NACL_SDK_ROOT + "/lib/pnacl/Release",
		NACL_SDK_ROOT + "/toolchain/usr/lib",
		NACL_SDK_ROOT + "/toolchain/linux_pnacl/usr/share/OpenCV/3rdparty/lib",
	],
	BUILDERS = {
		"PNaclBitcode": Builder(
			generator = generate_build_bitcode,
			suffix = '.bc',
			src_suffix = '.cpp')
	})

env['ENV']['TERM'] = os.environ['TERM']

# project specific code
env.PNaclBitcode(
	'hello_tutorial.bc',
	source = 
		'hello_tutorial.cpp',
	LIBS = [
		'ppapi_cpp',
		'ppapi',
		'opencv_calib3d',
		'opencv_highgui',
		'opencv_features2d',
		'opencv_imgproc',
		'opencv_flann',
		'opencv_core',
		'png',
		'z',
		'libjpeg',
		])

env.Command('hello_tutorial.pexe', 'hello_tutorial.bc', PNACL_TOOLCHAIN_PATH + '/bin/pnacl-finalize -o $TARGET $SOURCE')
