include ./make.conf

DIRS = src
DIRS += test

BUILDDIRS = $(DIRS:%=build-%)
DEBUGDIRS = $(DIRS:%=debug-%)
CLEANDIRS = $(DIRS:%=clean-%)

all: build

build: $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%) build

debug: $(DEBUGDIRS)
$(DEBUGDIRS):
	$(MAKE) -C $(@:debug-%=%) debug

clean: $(CLEANDIRS)
	rm -rf build

$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(DEBUGDIRS)
.PHONY: subdirs $(CLEANDIRS)
