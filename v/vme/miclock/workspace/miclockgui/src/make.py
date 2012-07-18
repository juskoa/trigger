import os
f = open("manifest", "w")
f.write("Manifest-Version: 1.0\nMain-Class: miclockGui.Main\nClass-Path: dim.jar miclockGui/\n")
f.close()
os.system("javac -cp dim.jar:miclockGui miclockGui/*.java")
os.system("jar cfm  MiClockGui.jar manifest miclockGui/*.class")
os.remove("manifest")
