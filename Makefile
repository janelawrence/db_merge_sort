CPPOPT=-g -Og -D_DEBUG
# -O2 -Os -Ofast
# -fprofile-generate -fprofile-use
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic
# -Wparentheses -Wno-unused-parameter -Wformat-security
# -fno-rtti -std=c++11 -std=c++98

# ensures that the linker includes the C++ standard library
LDLIBS=-lstdc++

# documents and scripts
DOCS=Tasks.txt
SCRS=

# headers and code sources
HDRS=	defs.h \
		Iterator.h Scan.h Filter.h Sort.h \
		Record.h HDD.h TreeOfLosers.h Run.h \
		CACHE.h \
SRCS=	defs.cpp Assert.cpp Test.cpp \
		Iterator.cpp Scan.cpp Filter.cpp Sort.cpp \
		Record.cpp HDD.cpp TreeOfLosers.cpp Run.cpp \
		CACHE.cpp \

# compilation targets
OBJS=	defs.o Assert.o Test.o \
		Iterator.o Scan.o Filter.o Sort.o \
		Record.o HDD.o TreeOfLosers.o Run.o \
		CACHE.o \

# RCS assists
REV=-q -f
MSG=no message

# default target
#
Test.exe : Makefile $(OBJS)
	g++ $(CPPFLAGS) -o Test.exe $(OBJS)

run: Test.exe
	./Test.exe $(numRecords) $(recordSize)

trace : Test.exe Makefile
	@date > trace
	./Test.exe >> trace
	@size -t Test.exe $(OBJS) | sort -r >> trace

# run_test : Test.exe Makefile
#     @./Test.exe arg1 arg2 arg3


$(OBJS) : Makefile defs.h
Test.o : Iterator.h Scan.h Filter.h Sort.h Record.h HDD.h TreeOfLosers.h Run.h CACHE.h 
CACHE.o: CACHE.h Run.h HDD.h
Run.o: Run.h Record.h
TreeOfLosers.o: Record.h TreeOfLosers.h
Iterator.o Scan.o Filter.o Sort.o : Iterator.h
Scan.o : Scan.h
Filter.o : Filter.h
Sort.o : Sort.h

list : Makefile
	echo Makefile $(HDRS) $(SRCS) $(DOCS) $(SCRS) > list
count : list
	@wc `cat list`

ci :
	ci $(REV) -m"$(MSG)" $(HDRS) $(SRCS) $(DOCS) $(SCRS)
	ci -l $(REV) -m"$(MSG)" Makefile
co :
	co $(REV) -l $(HDRS) $(SRCS) $(DOCS) $(SCRS)

clean :
	@rm -f $(OBJS) Test.exe Test.exe.stackdump trace
