"""
Command support for building dragon XML.
"""
from . import genericCommand


commandTemplate = """        <Command name="{name}" group="{group}" enabled="true" states="">\n
<description>{helpText}</description>
            <contents type="SCRIPT">
        <![CDATA[
        Sub Main
            DllCall "DictationBridgeClient32.dll","DB_SendCommand","{identifier_for_NVDA}"
        End Sub
        ]]>
            </contents>
        </Command>
"""

class DragonCommand(genericCommand.Command):
    """Command for building Dragon XML"""
    needsIO = True

    def open(self):
        return u"""<?xml version="1.0" encoding="utf-16"?>
	<!DOCTYPE MyCommands SYSTEM "http://www.nuance.com/NaturallySpeaking/Support/MyCommands/MyCmds11.dtd">
	<MyCommands version="2.0" language="0x409">
		<Commands type="global">
	"""

    def addCommand(self, command, identifier_for_jaws, identifier_for_NVDA, group, helpText):
        self.commandStr += commandTemplate.format(
            name = command,
            helpText=helpText,
            identifier_for_NVDA=identifier_for_NVDA,
            group = group
        )

    def finish(self):
        return """
        </Commands>
    </MyCommands>
        """
