#!/bin/bash

echo "se sterg fostele executabile (curatare)"
rm -f Server Senzor Sofer

echo "Se compileaza fisierele"

echo "gcc Server"
gcc Server.c -o Server
echo "gcc Senzor"
gcc Senzor.c -o Senzor
echo "gcc Sofer"
gcc Sofer.c -o Sofer -lncurses

echo "Se deschide serverul si senzorii"
gnome-terminal -- ./Server
sleep 5
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
gnome-terminal -- ./Senzor
