# Performance Decision Trees

Considering that for different problem classes different algorithms may
perform better, one problem that arises is how to automatically identify
these problem classes and suggest algorithm/parameter setting
configurations. This problem was well stated in the seminal work of Rice
(1976) ["The Algorithm Selection
Problem"](https://www.sciencedirect.com/science/article/pii/S0065245808605203).
Different machine learning approaches can be used to select algorithms
from
a [portfolio](http://robotics.stanford.edu/users/shoham/www%20papers/portfolio-IJCAI03.pdf).

This repository contains the code to produce [Decision
Trees](https://en.wikipedia.org/wiki/Decision_tree) for selecting
algorithms from the portfolio, here called *Performance Trees*. There are
two main advantages with this approach: (i) decision trees have high
interpretability, you can see in a hierarchical structure which features
are relevant to observe in your problem and (ii) once built, algorithm
recommendation can be provided in constant time.

## The code

The code is written in C++ and should compile in any compiler supporting
at least C++11.

### Building

Just run 

``` 
./configure
make
```

and the pdtree executable should build.

## Contact

Haroldo G. Santos - [haroldo@ufop.edu.br](haroldo@ufop.edu.br)

