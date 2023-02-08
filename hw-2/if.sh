#!/bin/bash

# some values for demonstration
int1='34';
int2='56';
str1='abc';
str2='abd';
intstr1='034';
file="if.sh"


if [ $str1 = $str2 ] # operator= tests whether values are equal as strings.
then
    echo '1.1. str1 is equal to str2';
fi # end the clause
if [ $str1 == $str2 ] # operator== is an alias for operator=
then
    echo '1.2. str1 is equal to str2';
fi

if [ $str1 != $str2 ] # operator!= tests whether values are not equal as strings.
then
    echo '2. str1 is not equal to str2';
fi

if [ ! $str1 = $str2 ] # !expression negates the expression
then
    echo '3. str1 is not equal to str2';
fi


if [ -n $str1 ] # -n tests whether the string has non-zero length
then
    echo '4. str1 is not empty';
fi
if [ -z $str1 ] # -n tests whether the string has zero length
then
    echo '5. str1 is empty';
fi



if [ $int1 -eq $int2 ] # -eq tests whether two values are equal as integers
then
    echo '6.1. int1 is equal to int2';
fi
# note that the results are different if comparing with -eq and with operator=
if [ $int1 -eq $intstr1 ] # as integers
then
    echo '6.2. int1 is equal to intstr1 as integer';
fi
if [ $int1 = $intstr1 ] # as strings
then
    echo '6.3. int1 is equal to intstr1 as string';
fi


if [ $int1 -gt $int2 ] # -eq tests whether first value is greater than second as integer
then
    echo '7. int1 is greater than int2';
fi
if [ $int1 -lt $int2 ] # -eq tests whether first value is less than second as integer
then
    echo '8. int1 is less than int2';
fi
# < and > can also be used here, but will require weird constructions like (("$a" < "$b")) and [[ "$a" < "$b" ]]



if [ $int1 -eq "34" ] # constants may also be used in conditions
then
    echo '9. int1 is 34';
fi


if [ -e "$file" ] # checks if the file exists
then
    echo '10. if.sh exists';
    
    if [ -d "$file" ] # checks if the file is a directory
    then
        echo '11. if.sh is a directory';
    else # else is supported
        echo '11. if.sh is not a directory';

        if [ -s "$file" ] # checks if the size of the file is greater than zero
        then
            echo '12. if.sh is not empty';
        else
            echo '12. if.sh is empty';
        fi
        
        if [ -r "$file" ] # checks if there is a read permission on file
        then
            echo '13. if.sh can be read';
        else
            echo '13. if.sh can not be read';
        fi

        if [ -w "$file" ] # checks if there is a write permission on file
        then
            echo '13. if.sh can be written to';
        else
            echo '13. if.sh cannot be written to';
        fi

        if [ -x "$file" ] # checks if there is an execute permission on file
        then
            echo '14. if.sh can be executed';
        else
            echo '14. if.sh cannot be executed';
        fi
    fi

else
    echo '10. if.sh does not exist';
fi