bin_PROGRAMS = mpdt selalg
mpdt_CPPFLAGS=-DCPX
mpdt_CXXFLAGS=-I/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/ilcplex/ -fPIC -m64 -fno-strict-aliasing
mpdt_LDADD=-L/opt/ibm/ILOG/CPLEX_Studio129/cplex/lib/x86-64_linux/static_pic -lcplex -lm -lpthread -ldl


selalg_CPPFLAGS=-DCPX
selalg_CXXFLAGS=-I/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/ilcplex/ -fPIC -m64 -fno-strict-aliasing
selalg_LDADD=-L/opt/ibm/ILOG/CPLEX_Studio129/cplex/lib/x86-64_linux/static_pic -lcplex -lm -lpthread -ldl

#mpdt_CPPFLAGS=-DGRB
#mpdt_CXXFLAGS=-I/opt/gurobi810/linux64/include/
#mpdt_LDFLAGS=-L/opt/gurobi810/linux64/lib/ -lgurobi81 -lm


#pdtree_SOURCES = pdtree.cpp \
#		 Dataset.cpp \
#		 InstanceSet.cpp \
#		 ResultsSet.cpp \
#		 Instance.cpp \
#		 FeatureBranching.cpp \
#		 Node.cpp \
#		 Tree.cpp \
#		 Parameters.cpp \
#		 SubSetResults.cpp \
#		 Branching.cpp \
#                ResTestSet.cpp \
#		 tinyxml2.cpp

#kfold_SOURCES =  kfold.cpp \
#		 Dataset.cpp \
#		 InstanceSet.cpp \
#		 ResultsSet.cpp \
#		 Instance.cpp \
#		 FeatureBranching.cpp \
#		 Node.cpp \
#		 Tree.cpp \
#		 Parameters.cpp \
#		 SubSetResults.cpp \
#		 Branching.cpp \
#                ResTestSet.cpp \
#		 tinyxml2.cpp

selalg_SOURCES = selalg.cpp \
		 MIPSelAlg.cpp \
                 lp.cpp \
		 Dataset.cpp \
		 ResultsSet.cpp \
		 Instance.cpp \
		 InstanceSet.cpp \
		 Parameters.cpp \
		 SubSetResults.cpp
		 

mpdt_SOURCES =  mpdt.cpp \
		lp.cpp \
		MIPPDtree.cpp \
		Dataset.cpp \
		InstanceSet.cpp \
		ResultsSet.cpp \
		Instance.cpp \
		Node.cpp \
		Tree.cpp \
		SubSetResults.cpp \
		Parameters.cpp \
		tinyxml2.cpp \
		Greedy.cpp

