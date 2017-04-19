#!/bin/bash

# This script is modeled on similar scripts provided by Dr. Sean Szumlanski 
# for COP 3503 Fall 2016 at UCF.

# Variable '$?' is the exit status of a command

# Compile project.c with spimcore.c to produce spimcore.
echo -n "Compiling project files... "
gcc -o spimcore spimcore.c project.c

compile_val=$?
if [ $compile_val == 0 ]; then
    echo "ok"
else
    echo "failed to compile"
    exit 1
fi

FILES="test-spim-rtype
test-spim-itype
test-spim-memry"

for f in $FILES
do
    # Provide some feedback
    echo -n "Checking $f.asc..."

    # Issue commands, followed by newline, saving output to file:
    # c to run all instructions
    # r to print registers
    # m to print memory
    # q to quit
    printf 'c\nr\nm\nh\nq\n' | ./spimcore $f.asc > $f-test-output.txt

    # Verify that program ran without error
    execute_val=$?
    if [ $execute_val != 0 ]; then
        echo "fail (program crashed)"
        exit 1
    fi

    # Compare expected output to output of program
    diff $f-output.txt $f-test-output.txt  > /dev/null

    # If output not the same print fail, else print PASS
    diff_val=$?
    if [ $diff_val != 0 ]; then
        echo "fail (output does not match)"
    else
        echo "PASS!"
    fi

    # Remove test output file
    rm $f-test-output.txt

done

exit 0
