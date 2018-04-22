import io
"""
A generic command creator. It receives a command and updates a command string. Then, when told to do so, it writes to the file it was created with.,
"""

class Command(object):
    needsIO = False 

    def __init__(self, filename):
        if self.needsIO:
            self.file = io.open(filename, "w", encoding="utf-16")
        else:
            self.file = open(filename, "w")
        self.commandStr = self.open()

    def close(self):
        self.commandStr += self.finish()
        self.file.write(self.commandStr)
        self.file.close()

    def open(self):
        raise NotImplementedError

    def addCommand(self, command, identifier_for_jaws, identifier_for_NVDA, group, helpText):
        """
        Adds a command to this command set.
        @param command: The command name, in <lang> lang.
        @param identifier_for_jaws: The Jaws identifier.
        @param identifier_for_NVDA: The NVDA identifier.
        @param group: The group, This is prefixed with "dictation Bridge" This makes the command have a hierarchy in the dictation solution.
        @param helpText: any additional help text the user should know about.
        """
        raise NotImplementedError

    def finish(self):
        raise NotImplementedError
