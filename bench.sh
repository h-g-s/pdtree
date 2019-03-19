export CXXFLAGS="-Ofast -flto -DNDEBUG" 
export LDFLAGS="-Ofast -flto" 
./configure ; make clean ; make
rm kfold.csv -f

export K=10

for mlb in 50 100 150 200 250 300;
do
./kfold ~/inst/algsel/rcpsp/features.csv ~/inst/algsel/rcpsp/results.csv $K rcpsp -minElementsBranch=${mlb}
./kfold ~/inst/algsel/cbcrelax/features.csv ~/inst/algsel/cbcrelax/results.csv $K cbcrelax -minElementsBranch=${mlb}
done

