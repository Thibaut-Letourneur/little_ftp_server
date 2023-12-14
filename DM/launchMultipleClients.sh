#!/bin/sh

if [ $# -lt 1 ];
then echo "Usage: ./launchMultipleClient <nbClients>"
exit
fi

nbLines=$(ps -xjf | grep -v 'grep' | grep 'PPID\|./serverD\|systemd/systemd' | wc -l)
if [ $nbLines -lt 3 ];#Le serveur n'est pas déjà lancé
then ./serverD &
fi

for i in $(seq 1 $1)
do
    ./client &
done
