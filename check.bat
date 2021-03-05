rem Use clang for error/warning check
clang -DDEBUG -I\mpir -Wall -Wno-char-subscripts -Wno-deprecated-declarations -Wno-string-plus-char -Wno-switch -Wno-unused-function -c *.cc
del *.o
