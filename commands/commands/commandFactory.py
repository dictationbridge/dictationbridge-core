from . import dragonCommand,wsrCommand,genericCommand,pythonCommand

class CommandFactory(object):
    def __init__(self):
        self.commands = [
            dragonCommand.DragonCommand("dragon_dictationBridgeCommands.xml"),
            pythonCommand.PythonCommand("NVDA_helpCommands.py"),
            wsrCommand.WSRCommand("dictationBridge.WSRMac"),
        ]

    def addCommand(self, *args, **kwargs):
        for i in self.commands:
            i.addCommand(*args, **kwargs)

    def close(self):
        for i in self.commands:
            i.close()