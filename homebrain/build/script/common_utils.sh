#!/bin/bash

__Check_and_Copy() {
    if [[ $# != 2 ]]
    then
        echo "Error: params invalid."
        exit
    fi
    if [[ -e $1 ]]
    then
        echo "cp -arpf $1 $2"
        cp -arpf $1 $2
    else
        echo "Error: $1 not exist!"
    fi
}

__Check_and_Link() {
    if [[ $# != 2 ]]
    then
        echo "Error: params invalid."
        exit
    fi
    if [[ -e $1 ]]
    then
        echo "ln -s $1 $2"
        ln -s $1 $2
    else
        echo "Error: $1 not exist!"
    fi
}
