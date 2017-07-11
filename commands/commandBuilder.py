from __future__ import print_function
"""
CommandBuilder builds commands for Dragon and WSR.
"""

from commands.dragonCommand import DragonCommand
from commands.wsrCommand import WSRCommand
import csv
import io
reader = None
try:
    commandsFile = open("commands.csv", "r")
    reader = csv.DictReader(commandsFile)
except (IOError,):
    print("Cannot read csv file")
    exit()
dragonCommandStr = u"""<?xml version="1.0" encoding="utf-16"?>
<!DOCTYPE MyCommands SYSTEM "http://www.nuance.com/NaturallySpeaking/Support/MyCommands/MyCmds11.dtd">
<MyCommands version="2.0" language="0x409">
    <Commands type="global">
"""
dragonFile = io.open("dragon_dictationBridgeCommands.xml", "w", encoding="utf-16")
WSRCommandStr = ""
WSRFile = open("dictationBridge.WSRMac", "w")
WSRFile.write("""<?xml version="1.0" encoding="UTF-8"?>
<speechMacros>
""")
for row in reader:
    commands = [row["speak"]] + row["shortSpeak"].split("|")
    #There are empty commands potentially if the alternates is empty, hence the if.
    #We want to strip accidental whitespace from around the command.
    commands = [command.strip() for command in commands if command]
    for command in commands:
        dragonCommand = DragonCommand(command)
        wsrCommand = WSRCommand(command)
        wsrCommand.helpText = dragonCommand.helpText = row["Notes"]
        if row["NVDA"]:
            wsrCommand.identifier_for_NVDA = dragonCommand.identifier_for_NVDA = row["NVDA"].strip()
        if row["jaws"]:
            wsrCommand.identifier_for_jaws = dragonCommand.identifier_for_jaws = row["jaws"].strip()
        dragonCommandStr += dragonCommand.makeCommand()
        WSRCommandStr += wsrCommand.makeCommand()

WSRFile.write(WSRCommandStr)
WSRFile.write("</speechMacros>")
WSRFile.close()
dragonCommandStr+="""
    </Commands>
</MyCommands>
"""
dragonFile.write(dragonCommandStr)
dragonFile.close()
