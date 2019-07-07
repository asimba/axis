include sources.inc

include $(CONFIG_BASE)

SRCDIR = .
BINDIR = ../bin
OBJDIR = ../obj

OBJEXT = o
OBJEXT_TOOLS = tmpo

OBJECTS=$(addprefix $(OBJDIR)/,$(SOURCES:.cc=.$(OBJEXT)))
OBJECTS_TOOLS=$(addprefix $(OBJDIR)/,$(SOURCES_TOOLS:.cc=.$(OBJEXT_TOOLS)))

all: clean tools $(PROGRAM)

clean-obj:
	@echo "Cleaning generated build files..."
	@rm -rf $(OBJDIR)/*

clean: clean-obj
	@echo "Cleaning old binaries..."
	@rm -f $(BINDIR)/$(PROGRAM)$(EXEEXT)

.SUFFIXES:
.SUFFIXES: .cc .$(OBJEXT) .$(OBJEXT_TOOLS)

tools: $(OBJECTS_TOOLS)
	@if ! test -f ./hashtbl.h ; then \
	echo "Linking   ($(BUILD_STRING)): $(notdir $(OBJDIR))/genhash$(EXEEXT)" ;\
	$(CC) $(LDFLAGS) $(OBJECTS_TOOLS) -o $(OBJDIR)/genhash$(EXEEXT) $(LFLAGS) ;\
	echo "Generating hash table..." ;\
	$(OBJDIR)/genhash$(EXEEXT) > ./hashtbl.h ; fi

$(PROGRAM): $(OBJECTS)
	@echo "Linking   ($(BUILD_STRING)): $(notdir $(BINDIR))/$(@F)$(EXEEXT)"
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $(BINDIR)/$@$(EXEEXT) $(LFLAGS)
	@strip -X -R .note -R .comment -R .note.gnu.build-id -R .note.ABI-tag $(BINDIR)/$@$(EXEEXT)

$(OBJDIR)/%.o:%.cc
	@echo "Compiling ($(BUILD_STRING)): $(notdir $(CURDIR))/$(<F)"
	@$(CC) -c $(CFLAGS) -DHAVE_HASH_TBL=1 $(INCLUDES) $< -o $@

$(OBJDIR)/%.tmpo:%.cc
	@if ! test -f ./hashtbl.h ; then \
	echo "Compiling ($(BUILD_STRING)): $(notdir $(CURDIR))/$(<F)" ;\
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ ; fi

.NOEXPORT:
