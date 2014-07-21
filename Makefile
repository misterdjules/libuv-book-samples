SOURCES := helloworld.c idle.c cat.c
OBJECTS := $(subst .c,.o,$(SOURCES))
SAMPLES := $(addprefix ./bin/,$(subst .c,,$(SOURCES)))

JSTRING_DIR := ./deps/jstring
JSTRING_STATIC_LIB := $(JSTRING_DIR)/libjstring.a
JSTRING_INCLUDE_DIR := $(JSTRING_DIR)

DEBUG_DIR := ./deps/debug
DEBUG_STATIC_LIB := $(DEBUG_DIR)/libdebug.a
DEBUG_INCLUDE_DIR := $(DEBUG_DIR)

LIBUV_DIR := ./deps/libuv

LDFLAGS += $(LIBUV_DIR)/libuv.a $(JSTRING_STATIC_LIB) $(DEBUG_STATIC_LIB) -lm -lrt -lpthread -lnsl -ldl
CFLAGS  += $(addprefix -I,$(LIBUV_DIR)/include $(JSTRING_DIR)) -g -O0 -Wall -pedantic

.PHONY: all

all: $(JSTRING_STATIC_LIB) $(DEBUG_STATIC_LIB) $(SAMPLES)

$(JSTRING_STATIC_LIB):
	$(MAKE) --directory=$(JSTRING_DIR)

$(DEBUG_STATIC_LIB):
	$(MAKE) --directory=$(DEBUG_DIR)

define SAMPLE_TEMPLATE
$1: $(notdir $(join $1,.c))
	$(CC) $(CFLAGS) $(notdir $(join $1,.c)) $(LDFLAGS) -o $1
endef

# Generates each test's binary executable using the template "TEST_TEMPLATE"
# defined above
$(foreach sample, $(SAMPLES), \
	$(eval $(call SAMPLE_TEMPLATE,$(sample))) \
)

clean:
	$(foreach sample, $(SAMPLES), \
		$(RM) $(sample) \
	)