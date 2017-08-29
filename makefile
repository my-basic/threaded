OS := $(shell uname -s)
IS_APPLE := $(shell echo $(OS)|grep -i darwin)

ifdef IS_APPLE
my_basic : threaded.o my_basic.o
	cc -o threaded threaded.o my_basic.o -lm
else
my_basic : threaded.o my_basic.o
	cc -o threaded threaded.o my_basic.o -lm -lrt
endif

threaded.o : threaded.c core/my_basic.h libs/thread.h
	cc -Os -c threaded.c -Wno-unused-result

my_basic.o : core/my_basic.c core/my_basic.h
	cc -Os -c core/my_basic.c -Wno-multichar -Wno-overflow -Wno-unused-result

clean :
	rm -f threaded.o my_basic.o threaded
