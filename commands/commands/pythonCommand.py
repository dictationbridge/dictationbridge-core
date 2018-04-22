"""
Command support for building dragon XML.
"""
from . import genericCommand
commandTemplate= "    {{'text':'{name}','helpText':'{helpText}','identifier_for_NVDA':'{identifier_for_NVDA}'}},"

class PythonCommand(genericCommand.Command):
    """Command for building a python dictionary"""

    def open(self):
        return u"commands=["

    def addCommand(self, command, identifier_for_jaws, identifier_for_NVDA, group, helpText):
        self.commandStr += commandTemplate.format(
            name = command.replace("'", "\\'"),
            helpText=helpText.replace("'", "\\'"),
            identifier_for_NVDA=identifier_for_NVDA,
        )

    def finish(self):
        return "]\n"
