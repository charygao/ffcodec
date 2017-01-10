# \file:	Makefile
# \brief:	dec264	
# \date:	2017.01.03
	
CXX      = g++
CXXFLAGS := -m64 -g -Wall -Wdisabled-optimization -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wuninitialized -Wunused-variable -Wunused-but-set-variable -Wunused-function
LIBVAR   += -lavcodec -lavutil
LIBPATH  := -Lffmpeglib/
TARGET   = ffdec

INCLUDES := includes
BIN_SRC  := Decoder.cpp

OUT_PATH  ?= out

CXXFLAGS += $(addprefix -I, $(INCLUDES))

#BIN_DEP  := $(addprefix $(OUT_PATH)/, $(BIN_SRC:.c=.d)) 
BIN_OBJ  := $(addprefix $(OUT_PATH)/, $(BIN_SRC:.cpp=.cpp.o))

#deps_rule = sed -e 's;^\([a-zA-Z0-9_]*\)\.o;${@:.d=.o} $@;'

all: $(TARGET) $(BIN_OBJ)

#ifneq ($(BIN_DEP),)
#$(BIN_DEP):$(OUT_PATH)/%.d:%
#	@echo "    [DEP]" $(notdir $(BIN_DEP))
#	@-mkdir -p $(dir $(BIN_DEP)) 
#	@$(CXX) $(CXXFLAGS) -M $< | $(deps_rule) >$@
#endif

ifneq ($(BIN_OBJ),)
$(BIN_OBJ):$(OUT_PATH)/%.o:%
	@echo "    [CC]" $(notdir $(BIN_OBJ))
	@-mkdir -p $(dir $(BIN_OBJ)) 
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
endif

ifneq ($(TARGET),)
$(TARGET):$(OUT_PATH)/$(TARGET)
$(OUT_PATH)/$(TARGET): $(BIN_OBJ)
	@echo "    [LD]" $(notdir $@)
	@-mkdir -p $(dir $@)
	@$(CXX) -o $@ $^ $(LIBVAR) $(LIBPATH)
endif

#run: build
#	./$(TARGET)

.PHONY: clean all
clean:
	rm -f $(OUT_PATH)/$(TARGET)
	rm -f $(BIN_OBJ)
#	rm -f $(BIN_DEP)