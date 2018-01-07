#!/bin/bash
# File a cura di Gemma Martini 532769
#Si dichiara che il contenuto del file è opera esclusiva della sottoscritta.
#In fede, Gemma Martini
usage="$(basename "$0") [-h] [file t] -- script to delete all files in file's folder that have been modified at least t minutes ago. If t is 0 no files are removed, but all their names are printed on standard output
        
where:
    -h  or --help show this help text
"

if [ "$#" -eq 0 ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "$usage"
else
    if [ "$#" -eq 2 ] 
    then
        myFile=$1
        myTime=$2
        myDir=$(grep 'DirName' $myFile | grep -v -e '#' | cut -d '=' -s -f 2)
        deletingFilesAndDirectories=$(find $myDir -mindepth 1 -mmin +$myTime)
        showingFiles=$(find $myDir -mindepth 1 -type f -mmin +$myTime)
        if [ "$myTime" -ne 0 ]
        then
            echo "Are you sure you want to delete all the following stuff?[Y/n] 

            $deletingFiles"
            read -r input
            if [ "$input" != "n" ]
            then
                find $myDir -mindepth 1 -mmin +$myTime -delete
                echo "Everything has been deleted"
            fi
        else
            echo "Here is the stuff contained in $myDir
         
            $showingFiles"
        fi
    else
         echo "error! See the usage -> $usage"
    fi
fi

