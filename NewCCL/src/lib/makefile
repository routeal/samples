include ../../make.conf

TARGET  = libccl

HEADERJJ = -I/Users/hwatanabe/Dropbox/routeal/ws/json-clib/include -I/Users/hwatanabe/Dropbox/routeal/ws/json-clib/lib

HEADERSS = ccl.h session.h port.h protocol.h
SOURCESS = session.c port.c protocol_17.c metadata.c

HEADERS = $(HEADERSS)
SOURCES = $(SOURCESS)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES)) 

CFLAGS  = $(COMMON_CFLAGS) $(HEADERJJ)
CFLAGS += -I../include -I./json/include
CFLAGS += -fPIC

ifeq ($(SYSTEM_NAME), Linux)
LDFLAGS += -shared -Wl,-soname,$(TARGET).so
endif

ifeq ($(SYSTEM_NAME), Darwin)
LDFLAGS += -dynamiclib -install_name @rpath/$(TARGET).so
endif

LIBS = /Users/hwatanabe/Dropbox/routeal/ws/json-clib/lib/libjson.a

all : build

build: CFLAGS += $(BUILD_CFLAGS)
build: $(TARGET).so $(TARGET).a

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(TARGET).so $(TARGET).a

$(TARGET).so: $(OBJECTS)
	${CC} $(LDFLAGS) $(LIBS) $^ -o $@ 

$(TARGET): $(OBJECTS)
	${CC} ${OBJECTS} -o $@ ${LDFLAGS} ${RPATH}

$(TARGET).a: $(OBJECTS)
	${AR} -rcs $@ $^ $(LIBS)

%.o:%.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

clean :
	rm -rf $(OBJECTS) $(TARGET).so $(TARGET).a core libjson.so.dSYM
