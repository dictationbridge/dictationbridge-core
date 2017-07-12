# Based on the main NVDA SCons script

import sys
import os
import time
import _winreg
from glob import glob
import commands.commandBuilder

def db_commandGenerator(target, source, env, for_signature):
	action = env.Action(lambda target, source, env : commands.commandBuilder.initialize() and None,
	lambda target, source, env : "Generating command files from {0} and building {1} and {2}".format(source[0], target[0], target[1]))
	return action

vars = Variables()

env = Environment(variables=vars,HOST_ARCH='x86',
	tools=[],
	toolpath = ['site_scons/site_tools']
)

binDir = Dir('bin')
Export('binDir')

outCommandFiles = [env.File(i) for i in [
	os.path.join("commands", "dragon_dictationBridgeCommands.xml"),
	os.path.join("commands", "dictationBridge.WSRMac"),
]]
sources = [
	env.File(os.path.join("commands", "commands.csv")),
	env.File(os.path.join("commands", "commandBuilder.py")),
]
sources += [env.File(i) for i in env.Glob("commands/commands/*.py")]

archTools=['default','windowsSdk','midl','msrpc']
env32=env.Clone(TARGET_ARCH='x86',tools=archTools)
env64=env.Clone(TARGET_ARCH='x86_64',tools=archTools)
# Hack around odd bug where some tool [after] msvc states that static and shared objects are different
env32['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
env64['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

env=env32

targets32 = env32.SConscript('archBuild_SConscript',exports={'env':env32,'binDir':binDir},variant_dir='build/x86')
targets64 = env64.SConscript('archBuild_SConscript',exports={'env':env64,'binDir':binDir},variant_dir='build/x86_64')
env['BUILDERS']['DB_CommandBuilder'] = Builder(generator=db_commandGenerator)
db_commands = env.DB_CommandBuilder( outCommandFiles, sources)


#We have to pass the list of targets out so that the NVDA add-on can depend.
Return('targets32 targets64 db_commands')
