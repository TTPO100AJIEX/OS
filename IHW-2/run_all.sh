for i in {4..10}
do
    echo "Testing $i"
    ./small.sh $i
    ./big.sh $i
done