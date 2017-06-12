include ../make.conf

DIRS += run_sessions

BUILDDIRS = $(DIRS:%=build-%)
DEBUGDIRS = $(DIRS:%=debug-%)
CLEANDIRS = $(DIRS:%=clean-%)

all: build

build: $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

debug: $(DEBUGDIRS)
$(DEBUGDIRS):
	$(MAKE) -C $(@:debug-%=%) debug

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(DEBUGDIRS)
.PHONY: subdirs $(CLEANDIRS)
