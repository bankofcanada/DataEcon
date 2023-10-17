# https://www.gnu.org/software/make/manual/make.html

# redirect all generated files to directory .cache
CACHEDIR = .cache
COVDIR = $(CACHEDIR)/cov
PROFDIR = $(CACHEDIR)/prof

# search path for targets and prereqs
VPATH = include src src/libdaec src/sqlite3 src/utils

# for C
CFLAGS = -std=c99 -O3 -g -Wall -Wpedantic -fPIC
# CFLAGS = -std=c99 -O0 -g -Wall -Wpedantic -fPIC

# for C++
CXXFLAGS = -std=c++11 -O3 -g -Wall -Wpedantic -fPIC
# CXXFLAGS = -std=c++11 -O0 -g -Wall -Wpedantic -fPIC

# add include directories from VPATH
CPPFLAGS = $(patsubst %,-I%,$(VPATH))

CFLAGS_COV = -fprofile-arcs -fprofile-abs-path -ftest-coverage 
CFLAGS_PROF = -pg -O0 -g

HAVE_READLINE = $(shell ./have_readline.sh $(CC))
HAVE_ZLIB = $(shell ./have_zlib.sh $(CC))

ifeq ($(target),)
# $(target) not specified => use OS we're running in
	ifeq ($(OS),Windows_NT)
		LIBDE = lib/daec.dll
		LIBDECOV = lib/daeccov.dll
		LIBDEPROF = lib/daecprof.dll
	else
		MY_LDFLAGS = -lpthread -ldl -lm
		LIBDE = lib/libdaec.so
		LIBDECOV = lib/libdaeccov.so
		LIBDEPROF = lib/libdaecprof.so
	endif
else
# We have $(target) - use it
	ifeq ($(findstring mingw32,$(target)),)
		MY_LDFLAGS = -lpthread -ldl -lm
		LIBDE = lib/libdaec.so
		LIBDECOV = lib/libdaeccov.so
		LIBDEPROF = lib/libdaecprof.so
	else
		LIBDE = lib/daec.dll
		LIBDECOV = lib/daeccov.dll
		LIBDEPROF = lib/daecprof.dll
	endif
endif 

# for the sqlite3 shell executable
SQLITE3 = bin/sqlite3
SQLITE3_SRC_H = sqlite3.h
SQLITE3_SRC_C = src/sqlite3/shell.c
SQLITE3_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(SQLITE3_SRC_C)))
SQLITE3_LDFLAGS = $(MY_LDFLAGS)

# for our library
# LIBDE = lib/libdaec.so
LIBDE_SRC_H = $(wildcard src/libdaec/*.h) sqlite3.h
LIBDE_SRC_C = $(wildcard src/libdaec/*.c)
LIBDE_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(LIBDE_SRC_C)))
LIBDE_LDFLAGS = $(MY_LDFLAGS)

LIBDEPROF_SRC_O = $(patsubst %.c,$(PROFDIR)/%.o,$(notdir $(LIBDE_SRC_C)))

# for library including coverage
# LIBDECOV = lib/libdaeccov.so
LIBDECOV_SRC_O = $(patsubst %.c,$(COVDIR)/%.o,$(notdir $(LIBDE_SRC_C)))
LIBDECOV_SRC_GCOV = $(patsubst %.c,%.c.gcov,$(notdir $(LIBDE_SRC_C)))

# for library instrumented for profiling
LIBDEPROF_SRC_O = $(patsubst %.c,$(PROFDIR)/%.o,$(notdir $(LIBDE_SRC_C)))

# for our shell executable (for now just an example how to use our library)
DESH = bin/desh
# DESH_SRC_H = daec.h $(wildcard src/*.h)
DESH_SRC_C = src/utils/desh.c
DESH_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(DESH_SRC_C)))
DESH_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDE))) -L lib -ldaec
ifeq ($(HAVE_READLINE),yes)
	DESH_CFLAGS += -DHAVE_READLINE
	DESH_LDFLAGS += -lreadline
endif
ifeq ($(HAVE_ZLIB),yes)
	DESH_CFLAGS += -DHAVE_ZLIB
	DESH_LDFLAGS += -lz
endif

DAEC2CSV = bin/daec2csv
DAEC2CSV_SRC_C = src/utils/daec2csv.c
DAEC2CSV_SRC_O =  $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(DAEC2CSV_SRC_C)))
DAEC2CSV_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDE))) -L lib -ldaec
ifeq ($(HAVE_ZLIB),yes)
	DAEC2CSV_CFLAGS += -DHAVE_ZLIB
	DAEC2CSV_LDFLAGS += -lz
endif

UTILS_COMMON_C = src/utils/common.c
UTILS_COMMON_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(UTILS_COMMON_C)))

PROF = bin/prof
# PROF_SRC_H = daec.h 
PROF_SRC_C = src/prof.c
PROF_SRC_O = $(patsubst %.c,$(PROFDIR)/%.o,$(notdir $(PROF_SRC_C)))
# PROF_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDE))) -L lib -ldaecprof

TEST = bin/test
TEST_SRC_C = src/test.c
TEST_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(TEST_SRC_C)))
TEST_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDE))) -L lib -ldaec

TESTCOV = bin/testcov
TESTCOV_SRC_C = src/test.c
TESTCOV_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(TEST_SRC_C)))
TESTCOV_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDECOV))) -L lib -ldaeccov

# default goal - build everything
all :: libdaec utils

# include auto-generated dependencies
include $(CACHEDIR)/Makefile.dep
include $(COVDIR)/Makefile.dep
include $(PROFDIR)/Makefile.dep

# auto-generate dependencies of each .o file on its .c and .h sources
$(CACHEDIR)/Makefile.dep: $(wildcard $(patsubst %,%/*.c,$(VPATH))) | $(CACHEDIR)
	@$(CC) $(patsubst %,-I%,$(VPATH)) -MM $^ > $@
	@sed -i -e 's,^\(.*\)\.o,$(CACHEDIR)/\1.o,g' $@

# auto-generate dependencies of each .o file on its .c and .h sources
$(COVDIR)/Makefile.dep: $(wildcard $(patsubst %,%/*.c,$(VPATH))) | $(COVDIR)
	@$(CC) $(patsubst %,-I%,$(VPATH)) -MM $^ > $@
	@sed -i -e 's,^\(.*\)\.o,$(COVDIR)/\1.o,g' $@

# auto-generate dependencies of each .o file on its .c and .h sources
$(PROFDIR)/Makefile.dep: $(wildcard $(patsubst %,%/*.c,$(VPATH))) | $(PROFDIR)
	@$(CC) $(patsubst %,-I%,$(VPATH)) -MM $^ > $@
	@sed -i -e 's,^\(.*\)\.o,$(PROFDIR)/\1.o,g' $@

# make .cache directory
$(CACHEDIR) :
	@mkdir -p $(CACHEDIR)

# make .cache/cov directory
$(COVDIR) : | $(CACHEDIR)
	@mkdir -p $(COVDIR)

# make .cache/prof directory
$(PROFDIR) : | $(CACHEDIR)
	@mkdir -p $(PROFDIR)

# make bin directory
bin :
	@mkdir -p bin

# make lib directory
lib :
	@mkdir -p lib

# redirect generated .o files into .cache
$(CACHEDIR)/%.o : %.c | $(CACHEDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# redirect generated .o files into .cache
$(COVDIR)/%.o : %.c | $(COVDIR)
	$(COMPILE.c) $(CFLAGS_COV) $(OUTPUT_OPTION) $<

# redirect generated .o files into .cache
$(PROFDIR)/%.o : %.c | $(PROFDIR)
	$(COMPILE.c) $(CFLAGS_PROF) $(OUTPUT_OPTION) $<

ifeq ($(HAVE_READLINE),yes)

# compile sqlite3 
$(SQLITE3_SRC_O) : $(SQLITE3_SRC_C) | $(CACHEDIR)
	$(COMPILE.c) -DHAVE_READLINE $(OUTPUT_OPTION) $<

# link sqlite3 shell executable
$(SQLITE3): $(SQLITE3_SRC_O) $(CACHEDIR)/sqlite3.o | bin
	$(LINK.c) $^ -o $@ $(SQLITE3_LDFLAGS) -lreadline

else 

# link sqlite3 shell executable
$(SQLITE3): $(SQLITE3_SRC_O) $(CACHEDIR)/sqlite3.o | bin
	$(LINK.c) $^ -o $@ $(SQLITE3_LDFLAGS)

endif

# link our library
$(LIBDE): $(LIBDE_SRC_O) $(CACHEDIR)/sqlite3.o | lib
	$(LINK.c) -shared $^ -o $@ $(LIBDE_LDFLAGS)

# link our library with coverage
$(LIBDECOV): $(LIBDECOV_SRC_O) $(CACHEDIR)/sqlite3.o | lib
	$(LINK.c) -lgcov --coverage -shared $^ -o $@ $(LIBDE_LDFLAGS)

# compile desh 
$(DESH_SRC_O) : $(DESH_SRC_C) | $(CACHEDIR)
	$(COMPILE.c) $(DESH_CFLAGS) $(OUTPUT_OPTION) $<

# link shell to our library (example)
$(DESH): $(DESH_SRC_O) $(UTILS_COMMON_O) | $(LIBDE) bin
	$(LINK.c) $^ -o $@ $(DESH_LDFLAGS)

# compile daec2csv 
$(DAEC2CSV_SRC_O) : $(DAEC2CSV_SRC_C) | $(CACHEDIR)
	$(COMPILE.c) $(DAEC2CSV_CFLAGS) $(OUTPUT_OPTION) $<

# link daec2csv
$(DAEC2CSV): $(DAEC2CSV_SRC_O) $(UTILS_COMMON_O) | $(LIBDE) bin
	$(LINK.c) $^ -o $@ $(DAEC2CSV_LDFLAGS)

# link profiling executable
$(PROF): $(PROF_SRC_O) $(LIBDEPROF_SRC_O) $(CACHEDIR)/sqlite3.o | bin
	$(LINK.c) -pg $^ -o $@ $(MY_LDFLAGS)

# link test executable with library with coverage
$(TEST): $(TEST_SRC_O) | $(LIBDE) bin
	$(LINK.c) $^ -o $@ $(TEST_LDFLAGS)

# link test executable with library with coverage
$(TESTCOV): $(TEST_SRC_O) | $(LIBDECOV) bin
	$(LINK.c) -lgcov --coverage $^ -o $@ $(TESTCOV_LDFLAGS)

# delete most generated files
.PHONY : clean
clean :: clean_cov clean_prof
	@rm -f $(LIBDE) $(LIBDE_SRC_O) $(TEST) $(TEST_SRC_O) $(DESH) $(DESH_SRC_O)
	@rm -f example example.daec

# delete all generated files
.PHONY : purge
purge :: clean
	@rm -f $(SQLITE3) $(SQLITE3_SRC_O)
	@rm -f -r $(CACHEDIR) bin lib

.PHONY : libdaec
libdaec :: $(LIBDE)

.PHONY : desh
desh :: $(DESH)

.PHONY : prof
prof :: $(PROF) | $(SQLITE3)
	rm -f prof.daec gmon.out prof.txt
	bash -c "time $(PROF)"
	gprof $(PROF) gmon.out > prof.txt
	$(SQLITE3) prof.daec 'select count(*) from objects'

.PHONY : sqlite3
sqlite3 :: $(SQLITE3)

.PHONY : utils
utils :: $(SQLITE3) $(DESH) $(DAEC2CSV)

# delete coverage files
.PHONY : clean_cov
clean_cov ::
	@rm -rf $(COVDIR)
	@rm -f $(LIBDECOV) $(TESTCOV) $(LIBDECOV_SRC_GCOV) lcov.info test.daec

# delete profiling files
.PHONY : clean_prof
clean_prof ::
	@rm -rf $(PROFDIR)
	@rm -f $(LIBDEPROF) $(PROF) prof.daec gmon.out prof.txt

.PHONY : test
test :: $(TEST) 
	bin/test

.PHONY : testcov
testcov :: $(TESTCOV) | $(COVDIR)
	bin/testcov

.PHONY : coverage
coverage :: testcov | $(COVDIR)
	gcov -o $(COVDIR) $(LIBDE_SRC_C) 
	lcov -c --directory $(COVDIR) -o lcov.info 

