bin_PROGRAMS = pdtree kfold mpdt
mpdt_CPPFLAGS=-DGRB
mpdt_CXXFLAGS=-I/opt/gurobi810/linux64/include/
mpdt_LDFLAGS=-L/opt/gurobi810/linux64/lib/ -lgurobi81 -lm

pdtree_SOURCES = pdtree.cpp \
		 Dataset.cpp \
		 InstanceSet.cpp \
		 ResultsSet.cpp \
		 Instance.cpp \
		 FeatureBranching.cpp \
		 Node.cpp \
		 Tree.cpp \
		 Parameters.cpp \
		 SubSetResults.cpp \
		 Branching.cpp \
                 ResTestSet.cpp \
		 tinyxml2.cpp

kfold_SOURCES =  kfold.cpp \
		 Dataset.cpp \
		 InstanceSet.cpp \
		 ResultsSet.cpp \
		 Instance.cpp \
		 FeatureBranching.cpp \
		 Node.cpp \
		 Tree.cpp \
		 Parameters.cpp \
		 SubSetResults.cpp \
		 Branching.cpp \
                 ResTestSet.cpp \
		 tinyxml2.cpp

mpdt_SOURCES =  mpdt.cpp \
		 lp.cpp \
		 MIPPDtree.cpp \
		 Dataset.cpp \
		 InstanceSet.cpp \
		 ResultsSet.cpp \
		 Instance.cpp \
		 FeatureBranching.cpp \
		 Node.cpp \
		 Tree.cpp \
		 Parameters.cpp \
		 SubSetResults.cpp \
		 Branching.cpp \
                 ResTestSet.cpp \
		 tinyxml2.cpp

