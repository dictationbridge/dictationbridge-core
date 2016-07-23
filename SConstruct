# Based on the main NVDA SCons script

import sys
import os
import time
import _winreg
from glob import glob

vars = Variables()

env = Environment(variables=vars,HOST_ARCH='x86',tools=[])

binDir = Dir('bin')
Export('binDir')

archTools=['default','windowsSdk','midl','msrpc']
env32=env.Clone(TARGET_ARCH='x86',tools=archTools)
env64=env.Clone(TARGET_ARCH='x86_64',tools=archTools)
# Hack around odd bug where some tool [after] msvc states that static and shared objects are different
env32['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
env64['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

env=env32

env32.SConscript('archBuild_SConscript',exports={'env':env32,'binDir':binDir},variant_dir='build/x86')
env64.SConscript('archBuild_SConscript',exports={'env':env64,'binDir':binDir},variant_dir='build/x86_64')
