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

### Prerequisites

* C++ compiler
* [Graphviz](https://www.graphviz.org/) to visualize the trees

### Building

Just run 

``` 
./configure
make
```

and the pdtree executable should build.

## Usage

pdtree receives as input two CSV files:

* _instance features_ file
* _experimental results_ file

The _instance features_ file should be as in this example:

```
instance,nProjects,avgStartProj,avgEndProj,nJobs
j3010_10.mm,1,3,3,32
J10053_3.mm,1,20,15,102
```

The first line contains the header. The first column should contain the
instance name and all other columns should contain features.

The _experimental results_ file should contain at least three columns: the
first column including the instance name, the last column including
a numerical result indicating the execution cost (e.g. processing time) of
the algorithm. Intermediate columns should indicate the algorithm and its
parameter settings as in the following example.

## Contact

Haroldo G. Santos - [haroldo@ufop.edu.br](haroldo@ufop.edu.br)

