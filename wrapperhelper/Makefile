all:
.PHONY: all

OPTIM?=2
DEBUG?=1
# FORCE_COLOR: set to non-empty, non 0 to force colorized output
# ECHO: set to non-empty, non 0 to echo commands out

help:
	@echo 'Targets:'
	@echo '  help (current target)'
	@echo '  all (default target)'
	@echo '  [EXE] - wrapperhelper'
	@printf ' $(foreach obj,$(OBJLIST_wrapperhelper), [OBJ] - $(obj)\n)'
	@printf ' $(foreach test,$(TESTS), [TST] - $(test)\n)'
	@echo '  clean'
	@echo '  distclean'
	@echo ''
	@echo 'Options:'
	@echo '  OPTIM: GCC optimization level (-O is prepended) [default: 2]'
	@echo '  DEBUG: set to 0 for release build, set to non-0 for debug build [default: 1]'
	@echo '  FORCE_COLOR: set to non-0 to force colorized output'
	@echo '  ECHO: set to non-0 to echo out commands executed'
	@echo ''
	@echo 'Current flags:'
	@echo '  CPPFLAGS = $(CPPFLAGS)'
	@echo '  CFLAGS   = $(CFLAGS)'
#	@echo '  CXXFLAGS = $(CXXFLAGS)' unused
	@echo '  LDFLAGS  = $(LDFLAGS)'
	@echo '  LDLIBS  = $(LDLIBS)'
	@echo ''
	@echo 'Sanitizers:'
	@echo "  address ------------ `[ $(ASAN_ON) -eq 1 ] && printf '\033[92mON\033[m' || printf '\033[91mOFF\033[m'`"
	@echo "  leak --------------- `[ $(LSAN_ON) -eq 1 ] && printf '\033[92mON\033[m' || printf '\033[91mOFF\033[m'`"
	@echo "  undefined behavior - `[ $(USAN_ON) -eq 1 ] && printf '\033[92mON\033[m' || printf '\033[91mOFF\033[m'`"
.PHONY: help

ifeq ($(ECHO:0=),)
SILENCER:=@
else
SILENCER:=
endif

ifneq ($(strip $(DEBUG)),0)
CPPFLAGS+= -DDEBUG -D_NRELEASE
CFLAGS+= -g
CXXFLAGS+= -g
LDFLAGS+= -g
OBJDIR?=debug
else
CPPFLAGS+= -DRELEASE -D_NDEBUG
OBJDIR?=release
endif

COMMON_WARNINGS:=-Wfatal-errors -fanalyzer -Wall -Wextra
COMMON_WARNINGS+= -Walloc-zero -Wcast-align=strict -Wcast-qual -Wconversion -Wdate-time
COMMON_WARNINGS+= -Wdisabled-optimization -Wduplicated-branches -Wfloat-equal -Wformat-truncation=2
COMMON_WARNINGS+= -Wimplicit-fallthrough=3 -Wlogical-op -Wmissing-format-attribute -Wmissing-include-dirs
COMMON_WARNINGS+= -Wmissing-noreturn -Wnull-dereference -Wredundant-decls -Wundef -Wunreachable-code -Wshift-overflow=2
COMMON_WARNINGS+= -Wstringop-overflow=4
#COMMON_WARNINGS+= -Wstringop-overflow=4 -Wsuggest-attribute=cold -Wsuggest-attribute=const -Wsuggest-attribute=format
#COMMON_WARNINGS+= -Wsuggest-attribute=malloc -Wsuggest-attribute=noreturn -Wsuggest-attribute=pure
COMMON_WARNINGS+= -Wunknown-pragmas -Wunused-macros -Wwrite-strings
COMMON_WARNINGS+= -Werror=attribute-alias=2 -Werror=duplicated-cond -Werror=format=2 -Werror=format-overflow=2
COMMON_WARNINGS+= -Werror=format-signedness -Werror=pointer-arith
COMMON_WARNINGS+= -Werror=return-type -Werror=shadow -Werror=strict-overflow -Werror=switch-enum
CFLAGS_WARNINGS:=-Werror=implicit-function-declaration -Werror=jump-misses-init -Werror=strict-prototypes
CXXFLAGS_WARNINGS:=-Werror=overloaded-virtual -fdiagnostics-show-template-tree -Wno-analyzer-use-of-uninitialized-value

CPPFLAGS+=
CFLAGS:=$(COMMON_WARNINGS) $(CFLAGS_WARNINGS) $(CFLAGS) -std=gnu18 -O$(OPTIM)
CXXFLAGS:=$(COMMON_WARNINGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS) -std=c++20 -O$(OPTIM)
LDFLAGS+= -O$(OPTIM)

#CPPFLAGS+= -I/usr/include/SDL2 -D_REENTRANT -pthread
#CFLAGS+= -pthread
#CXXFLAGS+= -pthread
#LDLIBS+= -pthread -lSDL2

ifeq (,$(wildcard $(CURDIR)/sanaddress))
ASAN_ON:=0
else
ASAN_ON:=1
CFLAGS+= -fsanitize=address
CXXFLAGS+= -fsanitize=address
LDFLAGS+= -fsanitize=address
endif
ifeq (,$(wildcard $(CURDIR)/sanleak))
LSAN_ON:=0
else
LSAN_ON:=1
CFLAGS+= -fsanitize=leak
CXXFLAGS+= -fsanitize=leak
LDFLAGS+= -fsanitize=leak
endif
ifeq (,$(wildcard $(CURDIR)/sanundefined))
USAN_ON:=0
else
USAN_ON:=1
CFLAGS+= -fsanitize=undefined
CXXFLAGS+= -fsanitize=undefined
LDFLAGS+= -fsanitize=undefined
endif

# Default
# .SUFFIXES: .out .a .ln .o .c .cc .C .cpp .p .f .F .m .r .y .l .ym .yl .s .S .mod
# .sym .def .h .info .dvi .tex .texinfo .texi .txinfo .w .ch .web .sh .elc .el
SUFFIXES =
.SUFFIXES:
.SECONDEXPANSION:

ifneq ($(MAKECMDGOALS:distclean=clean),clean)
.: ;
bin obj: ; $(SILENCER)test -d $@ || mkdir $@
# $(eval $(call reproduce_tree,<base>))
define reproduce_tree =
$(1): | $$$$(@D) ; $(SILENCER)test -d $$@ || mkdir $$@
endef
$(eval $(call reproduce_tree,obj/$(OBJDIR)))
$(eval $(call reproduce_tree,obj/$(OBJDIR)/tests))
$(eval $(call reproduce_tree,makedir))
$(eval $(call reproduce_tree,makedir/tests))
$(eval $(call reproduce_tree,tests))
endif

# Colors:
# -------
#   +--------+-----+
#   |   3    |  9  |
# +-+--------+-----+
# |0|        |     | Black
# |1|        | RM  | Red
# |2|        |[MSG]| Green
# |3|Creating|     | Yellow
# |4|        | CP  | Blue
# |5|        | LD  | Purple
# |6|  C++   |     | Cyan
# |7|        |     | Gray/white
# +-+--------+-----+

# $(call colorize,<br_color>,<br_text>,<text_color>,<text>)
ifdef $(if $(FORCE_COLOR:0=),FORCE_COLOR,MAKE_TERMOUT)
CFLAGS:=$(CFLAGS) -fdiagnostics-color
CXXFLAGS:=$(CFLAGS) -fdiagnostics-color
colorize=@printf "\033[$(1)m[$(2)]\033[m \033[$(3)m$(4)\033[m\n"
else
ifeq ($(SILENCER),)
colorize=
else
colorize=@echo "[$(2)] $(4)"
endif
endif

define newline :=


endef

# $(call remove,<list of file names to remove>)
define remove =
$(call colorize,1;91,RM ,91,Removing $(1))
$(SILENCER)$(RM) -r $(1)
endef

# $(eval $(call add_deptree,<compiler with flags>,<output_filename_noext>,<input_filename_withoutsrc>))
ifeq ($(MAKECMDGOALS),distclean)
add_deptree=
else
define add_deptree =
makedir/$(2).mk: | $$$$(@D)
	$(call colorize,95,DEP,33,Creating $(3) dependancies)
	$(SILENCER)set -e; $(1) -MM src/$(3) \
	 | sed 's,\($$(notdir $$(basename $(3)))\)\.o[ :]*,$$(dir obj/$(OBJDIR)/$(3))\1.o: $$@'"\n"'$$(dir obj/$(OBJDIR)/$(3))\1.o $$@: ,g' >$$@
include makedir/$(2).mk
endef
endif

OBJLIST=$(OBJLIST_wrapperhelper) $(foreach test,$(TESTS),$(call test_o,$(test)))
OBJLIST_wrapperhelper:=
TESTS:=

# $(call wrapperhelper_o,<base_dir>,<source_filename>,<output_filename>)
wrapperhelper_o=obj/$(OBJDIR)/$(1)$(3).o
# $(eval $(call compile_wrapperhelper_c,<base_dir>,<source_filename>,<output_filename>))
define compile_wrapperhelper_c =
$$(eval $$(call add_deptree,$$(CC) $$(CPPFLAGS) $$(CFLAGS),$(1)$(3),$(1)$(2).c))
OBJLIST_wrapperhelper+= $(call wrapperhelper_o,$(1),$(2),$(3))
$(call wrapperhelper_o,$(1),$(2),$(3)): src/$(1)$(2).c | $$$$(@D)
	$(call colorize,36, C ,92,Compiling $$@)
	$(SILENCER)$$(CC) $$(CPPFLAGS) $$(CFLAGS) -c src/$(1)$(2).c -o $$@
endef
# $(eval $(call compile_wrapperhelper_cxx,<base_dir>,<source_filename>,<output_filename>))
define compile_wrapperhelper_cxx =
$$(eval $$(call add_deptree,$$(CXX) $$(CPPFLAGS) $$(CXXFLAGS),$(1)$(3),$(1)$(2).cpp))
OBJLIST_wrapperhelper+= $(call wrapperhelper_o,$(1),$(2),$(3))
$(call wrapperhelper_o,$(1),$(2),$(3)): src/$(1)$(2).cpp | $$$$(@D)
	$(call colorize,36,C++,92,Compiling $$@)
	$(SILENCER)$$(CXX) $$(CPPFLAGS) $$(CXXFLAGS) -c src/$(1)$(2).cpp -o $$@
endef

# $(eval $(call compile_test_c,<test_dir/name>))
define compile_test_c =
$$(eval $$(call add_deptree,$$(CC) $$(CPPFLAGS) -Isrc/tests -Isrc $$(CFLAGS),tests/$(1),tests/$(1).c))
TESTS+= $(1)
tests/$(1): obj/$(OBJDIR)/tests/$(1).o | $$$$(@D)
	$(call colorize,95,LD ,92,Linking $$@)
	$(SILENCER)$$(CC) $$(LDFLAGS) -o $$@ obj/$(OBJDIR)/tests/$(1).o $$(LDLIBS)

obj/$(OBJDIR)/tests/$(1).o: src/tests/$(1).c | $$$$(@D)
	$(call colorize,36,C++,92,Compiling $$@)
	$(SILENCER)$$(CC) $$(CPPFLAGS) -Isrc/tests -Isrc $$(CFLAGS) -c src/tests/$(1).c -o $$@
endef
# $(eval $(call compile_test_cxx,<test_dir/name>))
define compile_test_cxx =
$$(eval $$(call add_deptree,$$(CXX) $$(CPPFLAGS) -Isrc/tests -Isrc $$(CXXFLAGS),tests/$(1),tests/$(1).cpp))
TESTS+= $(1)
tests/$(1): obj/$(OBJDIR)/tests/$(1).o | $$$$(@D)
	$(call colorize,95,LD ,92,Linking $$@)
	$(SILENCER)$$(CXX) $$(LDFLAGS) -o $$@ obj/$(OBJDIR)/tests/$(1).o $$(LDLIBS)

obj/$(OBJDIR)/tests/$(1).o: src/tests/$(1).cpp | $$$$(@D)
	$(call colorize,36,C++,92,Compiling $$@)
	$(SILENCER)$$(CXX) $$(CPPFLAGS) -Isrc/tests -Isrc $$(CXXFLAGS) -c src/tests/$(1).cpp -o $$@
endef

$(eval $(call compile_wrapperhelper_c,,cstring,cstring))
$(eval $(call compile_wrapperhelper_c,,generator,generator))
$(eval $(call compile_wrapperhelper_c,,lang,lang))
$(eval $(call compile_wrapperhelper_c,,log,log))
$(eval $(call compile_wrapperhelper_c,,machine,machine))
$(eval $(call compile_wrapperhelper_c,,main,main))
$(eval $(call compile_wrapperhelper_c,,parse,parse))
$(eval $(call compile_wrapperhelper_c,,prepare,prepare))
$(eval $(call compile_wrapperhelper_c,,preproc,preproc))
$(eval $(call compile_wrapperhelper_c,,vector,vector))
$(call wrapperhelper_o,,machine,machine): src/machine.gen
$(call wrapperhelper_o,,generator,generator): CFLAGS+= -fno-analyzer # Too slow
$(call wrapperhelper_o,,preproc,preproc): CFLAGS+= -fno-analyzer
$(call wrapperhelper_o,,parse,parse): CFLAGS+= -fno-analyzer

src/machine.gen:
	$(call colorize,96,GEN,33,Generating $@)
	$(SILENCER)echo | LC_ALL=C LANG=C $(CC) $(CPPFLAGS) -E -v - 2>&1 | sed ':l; $$ ! { N; b l }; s/.*#include <...> search starts here:\n//; s/End of search list.*//; s/^ /DO_PATH("/; s/\n /")\nDO_PATH("/g; s/\n$$/")/' >src/machine.gen

#$(eval $(call compile_test_cxx,core/number))

bin/wrapperhelper: $$(OBJLIST_wrapperhelper) | $$(@D)
	$(call colorize,95,LD ,92,Linking $@)
	$(SILENCER)$(CXX) $(LDFLAGS) -o $@ $(OBJLIST_wrapperhelper) $(LDLIBS)

wrapperhelper: bin/wrapperhelper
alltests: $(TESTS:%=tests/%)
.PHONY: wrapperhelper alltests

all: wrapperhelper alltests

clean:
	$(call remove,$(OBJLIST))
	$(call remove,bin/wrapperhelper)
	$(call remove,$(TESTS:%=obj/$(OBJDIR)/tests/%.o))
	$(call remove,$(TESTS:%=tests/%))
	$(call remove,src/machine.gen)
.PHONY: clean
distclean:
	$(call remove,makedir)
	$(call remove,obj)
	$(call remove,bin tests)
.PHONY: distclean

sanitize/help:
	@echo "Sanitizers:"
	@echo "- address (removes leak)"
	@echo "- leak (removes address)"
	@echo "- undefined behavior"
	@echo ""
	@echo "Currently active options:"
	@[ $(ASAN_ON) -eq 0 ] || echo "- address"
	@[ $(LSAN_ON) -eq 0 ] || echo "- leak"
	@[ $(USAN_ON) -eq 0 ] || echo "- undefined behavior"
sanitize/address:
	@[ $(ASAN_ON) -eq 0 ] && echo "Not sanitizing address" || echo "Sanitizing address"
sanitize/leak:
	@[ $(LSAN_ON) -eq 0 ] && echo "Not sanitizing leak" || echo "Sanitizing leak"
sanitize/undefined:
	@[ $(USAN_ON) -eq 0 ] && echo "Not sanitizing undefined behavior" || echo "Sanitizing undefined behavior"
sanitize/address/on:
	$(SILENCER)touch sanaddress
	$(SILENCER)$(RM) sanleak
sanitize/leak/on:
	$(SILENCER)touch sanleak
	$(SILENCER)$(RM) sanaddress
sanitize/undefined/on:
	$(SILENCER)touch sanundefined
sanitize/address/off:
	$(SILENCER)$(RM) sanaddress
sanitize/leak/off:
	$(SILENCER)$(RM) sanleak
sanitize/undefined/off:
	$(SILENCER)$(RM) sanundefined
.PHONY: sanitize/address     sanitize/leak     sanitize/undefined
.PHONY: sanitize/address/on  sanitize/leak/on  sanitize/undefined/on
.PHONY: sanitize/address/off sanitize/leak/off sanitize/undefined/off

tree:
	@tree src
.PHONY: tree

.DELETE_ON_ERROR:
