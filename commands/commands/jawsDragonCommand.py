"""
Command support for building dragon XML.
"""
from . import genericCommand


commandTemplate = """        <Command name="{name}" group="{group}" enabled="true" states="">\n
<description>{helpText}</description>
            <contents type="SCRIPT">
        <![CDATA[
        Sub Main
            Dim JAWS As New FSAPILib.JawsApi
            JAWS.RunScript "{identifier_for_jaws}"
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
        print identifier_for_jaws
        if not identifier_for_jaws:
            return
        self.commandStr += commandTemplate.format(
            name = command,
            helpText=helpText,
            identifier_for_jaws=identifier_for_jaws,
            group = group
        )

    def finish(self):
        return """
        </Commands>
    </MyCommands>
        """
