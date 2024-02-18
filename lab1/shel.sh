#!/bin/bash

# Script input param
DIRECTORY="$1"
ARGCOUNT=$#

# Inputs validation Code
CORRECT=0        # ОК!
NO_ARG_ERR=1     # No input parameters specified
DIRECTORY_ERR=2  # Incorrectly specified directory
MANY_ARG_ERR=3   # Many argument
HELP=4           # Print help

# Input parameter check function.
# Returns code that indicates the status of the check
function inputCheckStatus {

    local status=$CORRECT;

    if [ $ARGCOUNT -gt 1 ]; then
        status=$MANY_ARG_ERR
    fi

    if [ -z "$DIRECTORY" ]; then
        status=$NO_ARG_ERR
    else

        if [ "$DIRECTORY" == "-help" ]; then
            status=$HELP
        elif [ ! -d "$DIRECTORY" ]; then
            status=$DIRECTORY_ERR
        fi

    fi

    echo "$status"
}

# Function of displaying errors on the screen by their code.
# The output is carried out in stderr
function printErrorByStatus {
	err_code="$1"

	>&2 echo -n "Error: "
	case $err_code in 
		$NO_ARG_ERR) >&2 echo "directory not specified!";;
		$DIRECTORY_ERR) >&2 echo "\"$DIRECTORY\" is not a directory!";;
        $MANY_ARG_ERR) >&2 echo "Many argument pass, you must pass 1 arg: dirname";;
	esac
}


# Function of search and print directory without directorys
function dirSearch {
    cur_dir="$1"

    if [[ -z "$(/bin/ls -A $cur_dir 2>/dev/null)" ]]; then
   		echo " is empty folder"
	else
        dir_list="$(/usr/bin/find $cur_dir -maxdepth 1 -type d -name '*' -not -path $cur_dir 2>/dev/null)"
        if [[ -z "$dir_list" ]]; then
			echo " is empty folder"
		else
            for dir in $dir_list; do
                local nested_dir="$(/usr/bin/find $dir -maxdepth 1 -type d -name '*' -not -path $dir 2>/dev/null)"
                if [[ -z $nested_dir ]]; then
                    echo $dir
                else
                    dirSearch $dir # Recursive find subdirs
                fi
            done
        fi
    fi
}


# Main programm
input_status=$(inputCheckStatus)				# Getting inputs status

if [ "$input_status" -eq "$CORRECT" ]; then		# Status is correct
	dirSearch "$DIRECTORY"			# Start the main function
else
    if [ "$input_status" -eq "$HELP" ]; then
        echo "1) Enter the name of the directory for which you want to display folders that do not contain subdirectories."
        echo "2) If the directory you entered does not exist, an appropriate error message will be displayed."
        echo "3) If an incorrect number of parameters is entered (not equal to 1), an error message is displayed."
    else 											# Error status
	    $(printErrorByStatus "$input_status")		# Display the error 
    fi
fi