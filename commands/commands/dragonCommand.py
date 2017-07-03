"""
Command support for building dragon XML.
"""
import jinja2
from . import genericCommand


opener = '        <Command name="{name}" group="{group}" enabled="true" states="">\n'
command = """<description>{{helpText}}</description>
            <contents type="SCRIPT">
        <![CDATA[
        Function checkProcess(ProcessName, LoggedOnUser)
            'Check if the given process exists for the current logged on user
            'Returns true or false

            'Declare variables
            Dim objWMIService, colProcessList

            'Declare objects
            Set objWMIService = GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\c\imv2")
            Set colProcessList = objWMIService.ExecQuery("Select * from Win32_Process WHERE Name LIKE '" & ProcessName & "%'")

            'Check if process exists
            If colProcessList.Count>0 Then
                'The process exists, check if the process is running under the current logged on user
                For Each objProcess In colProcessList
                    objProcess.getOwner strUser
                    If strUser = LoggedOnUser Then
                        'Process found for currentlogged on user
                        'Return True
                        checkProcess = True
                    End If
                    Next
                Else
                'Process does not exists, return false
                checkProcess = False
            End If
            'Clean up objects
            Set objWMIService = Nothing
            Set colProcessList = Nothing
        End Function

        Function getCurrentLoggedonUser()
            'Get the username of the current logged on user
            'Returns the user name
            'Declare variable
            Dim objNetwork
            'Declare object
            Set objNetwork = CreateObject("WScript.Network")

            'Return the username
            getCurrentLoggedonUser = objNetwork.UserName

            'Clean up objects
            Set objNetwork = Nothing
        End Function

        Sub Main
            {% if identifier_for_NVDA and identifier_for_jaws %}
            If checkProcess("NVDA.exe",getCurrentLoggedonUser()) Then
                DllCall "DictationBridgeClient32.dll","DB_SendCommand","{{identifier_for_NVDA}}"
            Else
                If checkProcess("jfw.exe",getCurrentLoggedonUser()) Then
                    Set jaws=CreateObject("FreedomSci.JawsApi")
                    jaws.RunScript("{{identifier_for_jaws}}")
                    Set jaws=Nothing
                Else
                    MsgBox "Please run a supported screen reader and then speak this command again."
                End If
            End If
            {% elif identifier_for_NVDA and not identifier_for_jaws %}
            If checkProcess("NVDA.exe",getCurrentLoggedonUser()) Then
                DllCall "DictationBridgeClient32.dll","DB_SendCommand","{{identifier_for_NVDA}}"
            Else
                MsgBox "Please run NVDA and then speak this command again."
            End If
            {# We don't need to do another elif, because if not nvda only and not both, what else is it? #}
            {% else    %}
            If checkProcess("jfw.exe",getCurrentLoggedonUser()) Then
                Set jaws=CreateObject("FreedomSci.JawsApi")
                jaws.RunScript("{{identifier_for_jaws}}")
                Set jaws=Nothing
            Else
                MsgBox "Please run Jaws and then speak this command again."
            End If
            {% endif %}
        End Sub
        ]]>
            </contents>

"""
CLOSER = "        </Command>\n"

template = jinja2.Template(command)

class DragonCommand(genericCommand.Command):
    """Command for building Dragon XML"""

    def open(self):
        return opener.format(name=self.command, group=self.group)

    def commandMeat(self):
        return template.render(
            helpText=self.helpText,
            identifier_for_jaws=self.identifier_for_jaws,
            identifier_for_NVDA=self.identifier_for_NVDA,
        )

    def close(self):
        return CLOSER
