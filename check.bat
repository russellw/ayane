rem Use clang for error/warning check
clang -DDEBUG -I\mpir -Wall -Wno-deprecated-declarations -c *.cc
del *.o
