from . import NVDADragonCommand, jawsDragonCommand, wsrCommand,genericCommand,pythonCommand

class CommandFactory(object):
    def __init__(self):
        self.commands = [
            NVDADragonCommand.DragonCommand("dragon_dictationBridgeCommands.xml"),
            jawsDragonCommand.DragonCommand("jfw_dragon_dictationBridgeCommands.xml"),
            pythonCommand.PythonCommand("NVDA_helpCommands.py"),
            wsrCommand.WSRCommand("dictationBridge.WSRMac"),
        ]

    def addCommand(self, *args, **kwargs):
        for i in self.commands:
            i.addCommand(*args, **kwargs)

    def close(self):
        for i in self.commands:
            i.close()