function binary_search() # declare the function with code from while.sh except that variable "target" is changed to be $1 - the first and only parameter of the function
{
    left='0';
    right='1000000';
    while [ `expr $right - $left` -gt 1 ] # while right and left do not point to adjacent numbers, continue
    do
        mid=`expr $right + $left`; # sum left and right into mid
        mid=`expr $mid / 2`; # divide mid by 2

        if [ $1 -lt $mid ] # check if target is less than mid
        then
            # target is less than mid, thus the answer is not between mid and right
            echo "target is less than $mid";
            right=$mid; # move the right border
        else
            # target is greater or equal to mid, thus the answer is not between left and mid
            echo "target is not less than $mid";
            left=$mid; # move the left border
        fi
    done

    echo "target is $left"; # print the answer
    return $left;
}

binary_search "133457"; # call the function
binary_search "123"; # call the function