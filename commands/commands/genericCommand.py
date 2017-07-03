"""
A generic command creator. It receives a command and outputs a command string.
"""

class Command(object):

    def open(self):
        raise NotImplementedError

    def commandMeat(self):
        raise NotImplementedError

    def close(self):
        raise NotImplementedError

    def __init__(self, command, identifier_for_jaws=None, identifier_for_NVDA=None, group = "DictationBridge", helpText=""):
        self.command = command
        self.identifier_for_NVDA = identifier_for_NVDA
        self.identifier_for_jaws = identifier_for_jaws
        self.group = group
        self.helpText = helpText

    def makeCommand(self):
        open = self.open()
        commandMeat = self.commandMeat()
        close = self.close()
        return open+commandMeat+close