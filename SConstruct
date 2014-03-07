# The binary crashes when executed in browser.
# I suspect .cpp -> .o -> .bc -> .pexe conversion is wrong (make version is .cpp -> .bc -> .pexe)

# default header
import os

NACL_SDK_ROOT = "/home/xyx/nacl_sdk/pepper_33"

# Setup PNaCl tools
PNACL_TOOLCHAIN_PATH = NACL_SDK_ROOT + "/toolchain/linux_pnacl"

env = Environment(
	CXX = PNACL_TOOLCHAIN_PATH + "/bin/pnacl-clang++",
	CXXFLAGS = '-std=gnu++98 -pthread -Wno-long-long -Wall -Wswitch-enum -pedantic -Werror',
	CCFLAGS = ['-O2'],
	CPPPATH = [
		NACL_SDK_ROOT + "/include",
		PNACL_TOOLCHAIN_PATH + "/usr/include",
	],
	LIBPATH = [
		NACL_SDK_ROOT + "/lib/pnacl/Release",
		NACL_SDK_ROOT + "/toolchain/usr/lib",
		NACL_SDK_ROOT + "/toolchain/linux_pnacl/usr/share/OpenCV/3rdparty/lib",
	])

env['ENV']['TERM'] = os.environ['TERM']

# project specific code
env.Program(
	'hello_tutorial.bc',
	source = 
		'hello_tutorial.cpp',
	LIBS = [
		'ppapi_cpp',
		'ppapi',
		'opencv_highgui',
		'opencv_imgproc',
		'opencv_core',
		'png',
		'z',
		'libjpeg',
		])

env.Command('hello_tutorial.pexe', 'hello_tutorial.bc', PNACL_TOOLCHAIN_PATH + '/bin/pnacl-finalize -o $TARGET $SOURCE')
