from . import genericCommand
commandTemplate = """
    <command>
        <listenFor>{command}</listenFor>
        <run command="msr_util" params="{identifier_for_NVDA}"/>
    </command>"""

class WSRCommand(genericCommand.Command):

    def open(self):
        return """<?xml version="1.0" encoding="UTF-8"?>
<speechMacros>"""

    def addCommand(self, command, identifier_for_jaws, identifier_for_NVDA, group, helpText):
        self.commandStr += commandTemplate.format(
            command=command,
            identifier_for_NVDA=identifier_for_NVDA,
        )

    def finish(self):
        return "\n</speechMacros>"