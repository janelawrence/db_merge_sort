CPPOPT=-g -Og -D_DEBUG
# -O2 -Os -Ofast
# -fprofile-generate -fprofile-use
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic -std=c++11
# -Wparentheses -Wno-unused-parameter -Wformat-security
# -fno-rtti -std=c++11 -std=c++98

# ensures that the linker includes the C++ standard library
LDLIBS=-lstdc++

# documents and scripts
DOCS=Tasks.txt
SCRS=

# headers and code sources
HDRS=	defs.h \
		Record.h Disk.h TreeOfLosers.h Run.h \
		CACHE.h DRAM.h Page.h TournamentTree.h\
SRCS=	defs.cpp Assert.cpp Test.cpp \
		Record.cpp Disk.cpp TreeOfLosers.cpp Run.cpp \
		CACHE.cpp DRAM.cpp Page.cpp TournamentTree.cpp\

# compilation targets
OBJS=	defs.o Assert.o Test.o \
		Record.o Disk.o TreeOfLosers.o Run.o \
		CACHE.o DRAM.o Page.o TournamentTree.o\

# RCS assists
REV=-q -f
MSG=no message

TARGET = ExternalSort.exe

# default target
#
$(TARGET) : Makefile $(OBJS)
	g++ $(CPPFLAGS) $(OBJS) -o $@

run: $(TARGET)
	./$(TARGET) -c 20 -s 1024 -o trace0.txt

# trace : Test.exe Makefile
# 	@date > trace
# 	./Test.exe >> trace
# 	@size -t Test.exe $(OBJS) | sort -r >> trace

# run_test : Test.exe Makefile
#     @./Test.exe arg1 arg2 arg3


$(OBJS) : Makefile defs.h
Test.o : Record.h Disk.h TreeOfLosers.h Run.h CACHE.h DRAM.h Page.h TournamentTree.h
Page.o: Page.h Record.h
Run.o: Run.h Record.h Page.h
CACHE.o: CACHE.h Run.h Disk.h
DRAM.o: defs.h Run.h TreeOfLosers.h Disk.h
TournamentTree.o: TournamentTree.h Record.h Run.h
TreeOfLosers.o: Record.h Page.h TreeOfLosers.h


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
