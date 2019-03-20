export CXXFLAGS="-Ofast -flto -DNDEBUG" 
export LDFLAGS="-Ofast -flto" 
cd ..
./configure ; make clean ; make
rm kfold.csv -f
mv kfold bench
cd bench

export K=10

for depth in 1 2 3 4 5;
do
    for mlb in 50 100 150 200 250 300;
    do
        ./kfold ~/inst/algsel/rcpsp/features.csv ~/inst/algsel/rcpsp/results.csv $K rcpsp -maxDepth=${depth} -minElementsBranch=${mlb} -fmrValue=1.0
        ./kfold ~/inst/algsel/cbcrelax/features.csv ~/inst/algsel/cbcrelax/results.csv $K cbcrelax -maxDepth=${depth} -minElementsBranch=${mlb} -fmrValue=8000
    done
done

