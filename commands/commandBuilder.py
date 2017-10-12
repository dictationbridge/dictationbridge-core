from __future__ import print_function
import os

"""
CommandBuilder builds commands for Dragon and WSR.
"""

from commands.commandFactory import CommandFactory
import csv
import io
reader = None
def initialize():
	origDir = os.path.abspath(os.curdir)
	os.chdir(os.path.dirname(__file__))
	try:
		commandFile = open("commands.csv")
		reader = csv.DictReader(commandFile)
	except (IOError,):
		print("Cannot read csv file")
		exit()
	commandFactorie = CommandFactory()
	for row in reader:
		commands = [row["speak"]] + row["shortSpeak"].split("|")
		#There are empty commands potentially if the alternates is empty, hence the if.
		#We want to strip accidental whitespace from around the command.
		commands = [command.strip() for command in commands if command]
		group = row.get("group", "").strip()
		if group:
			group = "Dictation Bridge/" + group
		else:
			group = "Dictation Bridge"
		for command in commands:
			commandFactorie.addCommand(command,
				row.get("jaws", "").strip(),
				row.get("NVDA", "").strip(), 
				group,
				row.get("Notes", "").strip())
	commandFactorie.close()
	commandFile.close()
	os.chdir(origDir)

