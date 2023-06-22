# https://www.gnu.org/software/make/manual/make.html

# redirect all generated files to directory .cache
CACHEDIR = .cache
COVDIR = $(CACHEDIR)/cov

# search path for targets and prereqs
VPATH = src src/libdaec src/sqlite3

# vpath %.h src/libdaec src/sqlite3
# vpath %.c src/libdaec src/sqlite3
# vpath %.o src/libdaec src/sqlite3

# CFLAGS = -std=c99 -O3 -Wall -Wpedantic -fPIC
CFLAGS = -std=c99 -O0 -g -Wall -Wpedantic -fPIC
# add include directories from VPATH
CFLAGS += $(patsubst %,-I%,$(VPATH))

CFLAGS_COV = -fprofile-arcs -fprofile-abs-path -ftest-coverage 

ifeq ($(findstring mingw32,$(target)),)
    MY_LDFLAGS = -lpthread -ldl -lm
	LIBDE = bin/libdaec.so
	LIBDECOV = bin/libdaeccov.so
else
	LIBDE = bin/daec.dll
	LIBDECOV = bin/daecCOV.dll
endif

# for the sqlite3 shell executable
SQLITE3 = bin/sqlite3
SQLITE3_SRC_H = sqlite3.h
SQLITE3_SRC_C = src/sqlite3/sqlite3.c src/sqlite3/shell.c
SQLITE3_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(SQLITE3_SRC_C)))
SQLITE3_LDFLAGS = $(MY_LDFLAGS)

# for our library
# LIBDE = bin/libdaec.so
LIBDE_SRC_H = $(wildcard src/libdaec/*.h) sqlite3.h
LIBDE_SRC_C = $(wildcard src/libdaec/*.c)
LIBDE_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(LIBDE_SRC_C)))
LIBDE_LDFLAGS = $(MY_LDFLAGS)

# for library including coverage
# LIBDECOV = bin/libdaeccov.so
LIBDECOV_SRC_O = $(patsubst %.c,$(COVDIR)/%.o,$(notdir $(LIBDE_SRC_C)))
LIBDECOV_SRC_GCOV = $(patsubst %.c,%.c.gcov,$(notdir $(LIBDE_SRC_C)))

# for our shell executable (for now just an example how to use our library)
DESH = bin/desh
# DESH_SRC_H = daec.h $(wildcard src/*.h)
DESH_SRC_C = src/desh.c
DESH_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(DESH_SRC_C)))
DESH_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDE))) -L bin -ldaec

TEST = bin/test
TEST_SRC_C = src/test.c
TEST_SRC_O = $(patsubst %.c,$(CACHEDIR)/%.o,$(notdir $(TEST_SRC_C)))
TEST_LDFLAGS = -Wl,-rpath,$(abspath $(dir $(LIBDECOV))) -L bin -ldaeccov

# default goal - build everything
all :: lib sqlite3

# include auto-generated dependencies
include $(CACHEDIR)/Makefile.dep
include $(COVDIR)/Makefile.dep

# auto-generate dependencies of each .o file on its .c and .h sources
$(CACHEDIR)/Makefile.dep: $(wildcard $(patsubst %,%/*.c,$(VPATH))) | $(CACHEDIR)
	@$(CC) $(patsubst %,-I%,$(VPATH)) -MM $^ > $@
	@sed -i -e 's,^\(.*\)\.o,$(CACHEDIR)/\1.o,g' $@

# auto-generate dependencies of each .o file on its .c and .h sources
$(COVDIR)/Makefile.dep: $(wildcard $(patsubst %,%/*.c,$(VPATH))) | $(COVDIR)
	@$(CC) $(patsubst %,-I%,$(VPATH)) -MM $^ > $@
	@sed -i -e 's,^\(.*\)\.o,$(COVDIR)/\1.o,g' $@

# make .cache directory
$(CACHEDIR) :
	@mkdir -p $(CACHEDIR)

# make .cache/cov directory
$(COVDIR) : | $(CACHEDIR)
	@mkdir -p $(COVDIR)

# make bin directory
bin :
	@mkdir -p bin

# redirect generated .o files into .cache
$(CACHEDIR)/%.o : %.c | $(CACHEDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# redirect generated .o files into .cache
$(COVDIR)/%.o : %.c | $(COVDIR)
	$(COMPILE.c) $(CFLAGS_COV) $(OUTPUT_OPTION) $<

# link sqlite3 shell executable
$(SQLITE3): $(SQLITE3_SRC_O) | bin
	$(LINK.c) $^ -o $@ $(SQLITE3_LDFLAGS)

# link our library
$(LIBDE): $(LIBDE_SRC_O) $(CACHEDIR)/sqlite3.o | bin
	$(LINK.c) -shared $^ -o $@ $(LIBDE_LDFLAGS)

# link our library with coverage
$(LIBDECOV): $(LIBDECOV_SRC_O) $(CACHEDIR)/sqlite3.o | bin
	$(LINK.c) -lgcov --coverage -shared $^ -o $@ $(LIBDE_LDFLAGS)

# link shell to our library (example)
$(DESH): $(DESH_SRC_O) | $(LIBDE) bin
	$(LINK.c) $^ -o $@ $(DESH_LDFLAGS)

# link test executable with library with coverage
$(TEST): $(TEST_SRC_O) | $(LIBDECOV) bin
	$(LINK.c) -lgcov --coverage $^ -o $@ $(TEST_LDFLAGS)

# delete most generated files
.PHONY : clean
clean :: clean_cov
	@rm -f $(LIBDE) $(LIBDE_SRC_O) $(DESH) $(DESH_SRC_O)

# delete all generated files
.PHONY : purge
purge :: clean
	@rm -f $(SQLITE3) $(SQLITE3_SRC_O)
	@rm -f -r $(CACHEDIR)

.PHONY : lib
lib :: $(LIBDE)

.PHONY : desh
desh :: $(DESH)

.PHONY : sqlite3
sqlite3 :: $(SQLITE3)

# delete coverage files
.PHONY : clean_cov
clean_cov ::
	@rm -rf $(COVDIR)
	@rm -f $(LIBDECOV) $(TEST) $(TEST_SRC_O) $(LIBDECOV_SRC_GCOV) lcov.info test.daec

.PHONY : test
test :: $(TEST) | $(COVDIR)
	bin/test

.PHONY : coverage
coverage :: test | $(COVDIR)
	gcov -o $(COVDIR) $(LIBDE_SRC_C) 
	lcov -c --directory $(COVDIR) -o lcov.info 

