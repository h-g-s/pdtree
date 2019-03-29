export CXXFLAGS="-O0 -Og -g3 -DDEBUG -fsanitize=address -D_GLIBCXX_DEBUG" ; export LFFLAGS="-g -fsanitize=address" ; ./configure ; make clean ; make
