"""
Command support for building dragon XML.
"""
from . import genericCommand


opener = '        <Command name="{name}" group="{group}" enabled="true" states="">\n'
command = r"""<description>{helpText}</description>
            <contents type="SCRIPT">
        <![CDATA[
        Sub Main
            DllCall "DictationBridgeClient32.dll","DB_SendCommand","{identifier_for_NVDA}"
        End Sub
        ]]>
            </contents>
"""
CLOSER = "        </Command>\n"


class DragonCommand(genericCommand.Command):
    """Command for building Dragon XML"""

    def open(self):
        return opener.format(name=self.command, group=self.group)

    def commandMeat(self):
        return command.format(
            helpText=self.helpText,
            identifier_for_jaws=self.identifier_for_jaws,
            identifier_for_NVDA=self.identifier_for_NVDA,
        )

    def close(self):
        return CLOSER
