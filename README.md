# DictationBridge Core

This is the screen-reader-independent core of DictationBridge. Its primary purpose is to detect text input via Windows Speech Recognition and Dragon NaturallySpeaking, so the screen-reader-specific add-on can echo back that text.

Support for Dragon NaturallySpeaking is still under heavy development.

## Building

To build the core, you need Python 2.7, a recent version of SCons, and Visual Studio 2015.

First, fetch Git submodules with this command:

    git submodule update --init --recursive

Then simply run SCons.

## Copyright and license

Copyright 2016 3 Mouse Technology, LLC. Based on code licensed from Serotek Corporation. This software is provided under the Mozilla Public License, version 2.0.
