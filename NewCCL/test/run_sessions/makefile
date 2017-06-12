include ../../make.conf

TARGET = run_sessions

SOURCES  = main.c
OBJECTS  = $(patsubst %.c,%.o,$(SOURCES))

CFLAGS  += -I../../src/include -I../../src/lib -I/Users/hwatanabe/Dropbox/routeal/ws/json-clib/include
LDFLAGS += -L../../src/lib -lccl

CFLAGS  += $(COMMON_CFLAGS)
LDFLAGS += $(COMMON_LDFLAGS)

all : $(TARGET)

build: CFLAGS += $(BUILD_CFLAGS)
build: $(TARGET)

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(TARGET)

ifeq ($(SYSTEM_NAME), Linux)
RPATH = -Wl,-rpath,../../src/lib
LDFLAGS += -pthread
endif
ifeq ($(SYSTEM_NAME), Darwin)
RPATH = -Xlinker -rpath -Xlinker ../../src/lib
endif

$(TARGET): $(OBJECTS)
	${CC} ${OBJECTS} -o $@ ${LDFLAGS} ${RPATH}

$(BUILDDIR)/%.o:%.c $(BUILDDIR)/.d
	${CC} ${CFLAGS} -c $< -o $@

%/.d:
	mkdir -p $(BUILDDIR)

clean :
	rm -f $(OBJECTS) $(TARGET) $(BUILDDIR)/core
