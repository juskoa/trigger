#!/bin/bash
echo -e "Manifest-Version: 1.0\nMain-Class: miclockGui.Main\nClass-Path: dim.jar miclockGui/" >manifest
javac -cp dim.jar:miclockGui miclockGui/*.java
jar cfm  MiClockGui.jar manifest miclockGui/*.class
rm manifest
