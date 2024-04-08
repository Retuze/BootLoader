######################################
# 预定义的一些函数
######################################
# brief : 打印编译消息
# eg : $(call print,compiling,file)
ifeq ($(OS),Windows_NT)
	print=echo $(1) $(notdir $(2)) $(3)
else
	print=printf "$(1) $(notdir $(2)) $(3)"
endif
# brief : 递归遍历指定后缀的文件
# eg : $(call rwildcard,path,suffix)
rwildcard = $(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2))$(filter $(subst *,%,$(2)),$(d)))

# brief : 删除文件或者文件夹
# eg : $(RM) file
ifeq ($(OS),Windows_NT)
	RM = del /Q
else
	RM = rm -rf
endif

# brief : 是否打开调试输出信息（默认关闭）
ifeq ($(Debug),0)
	AT = 
else 
	AT = @
endif

######################################
# target

######################################
TARGET = BootLoader


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################

# binaries
#######################################
# 排除文件夹
FILTER = docs
BUILD_FILE = . 
# 编译文件夹 默认为工作目录加库目录
SOURCES = $(filter-out $(FILTER),$(foreach d,$(BUILD_FILE),$(wildcard $(d)/*)))
# C sources
C_SOURCES += $(sort $(call rwildcard,$(SOURCES),*.c)) 
# CPP sources
CPP_SOURCES += $(sort $(call rwildcard,$(SOURCES),*.cpp))
# ASM sources
ASM_SOURCES += $(sort $(call rwildcard,$(SOURCES),*.s)) 
ASM_SOURCESS += $(sort $(call rwildcard,$(SOURCES),*.S))
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CPP = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
CPP = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS


#######################################
# cpu
CPU = -mcpu=cortex-m3

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F103xB




# compile gcc flags
C_INCLUDES += $(sort $(foreach d,$(call rwildcard,$(SOURCES),*.h),$(dir -I$d)))
C_INCLUDES += $(sort $(foreach d,$(call rwildcard,$(SOURCES),*.hpp),$(dir -I$d)))

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script

LDSCRIPT = ldscript.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
LDFLAGS += -lstdc++ -Wl,--no-warn-rwx-segments
# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of C objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of CPP objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCESS:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCESS)))

$(BUILD_DIR)/%.o: %.c mk.mk | $(BUILD_DIR) 
	$(AT)$(call print,compiling,$<,...)
	$(AT)$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
$(BUILD_DIR)/%.o: %.cpp mk.mk | $(BUILD_DIR) 
	$(AT)$(call print,compiling,$<,...)
	$(AT)$(CPP) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@
$(BUILD_DIR)/%.o: %.s mk.mk | $(BUILD_DIR)
	$(AT)$(call print,assebmling,$<,...)
	$(AT)$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S mk.mk | $(BUILD_DIR)
	$(AT)$(call print,assebmling,$<,...)
	$(AT)$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) mk.mk
	$(AT)$(call print,linking,$@,...)
	$(AT)$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(AT)$(call print,successful build,$@,)
	$(AT)$(SZ) $@
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(AT)$(HEX) $< $@
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(AT)$(BIN) $< $@	
$(BUILD_DIR):
ifeq ($(OS),Windows_NT)
	$(AT)-mkdir $(BUILD_DIR)
else
	$(AT)$(shell mkdir $(BUILD_DIR) 2>/dev/null)
endif		

#######################################
# clean up
#######################################
clean:
ifeq ($(OS),Windows_NT)
	$(AT)-$(RM) $(subst /,\\,$(BUILD_DIR)/)
else
	$(AT)-$(RM) $(BUILD_DIR)/*
endif	
	
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***