from . import genericCommand
import jinja2
command = """
        <listenFor>{{command}}</listenFor>
        <run command="msr_util" params="{{identifier_for_NVDA}}"/>

"""
template = jinja2.Template(command)
class WSRCommand(genericCommand.Command):

    def open(self):
        return "    <command>"

    def commandMeat(self):
        return template.render(
            command=self.command,
            identifier_for_NVDA=self.identifier_for_NVDA,
        )

    def close(self):
        return "    </command>\n"
