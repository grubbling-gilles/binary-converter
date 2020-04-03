cmake_minimum_required(VERSION 3.15)
project("Binary Converter")

#Set name for final output
set(PRODUCT_NAME binary_converter)

SET(PORT "/dev/ttyACM0")

# MicroController Unit (MCU)
SET(MCU "atmega328p")
# set processor clock speed
SET(F_CPU "16000000")

#Set baud rate
SET (BAUD 9600)

# The programmer to use -- programmer refers to device used to load code into chip?
# use command `avrdude -c ?` to print all available programmer
set(PROG_TYPE arduino)

#Geen idee wat dit is ??
# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p
#set(E_FUSE 0xfd)
#set(H_FUSE 0xda)
#set(L_FUSE 0xfd)
#set(LOCK_BIT 0xff)

# Use AVR GCC toolchain
SET(CMAKE_C_COMPILER /usr/bin/avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Add definitions here -- no need to define these in source code
add_definitions(
        -DF_CPU=${F_CPU}
        -DBAUD=${BAUD}
)

# mmcu MUST be passed to bot the compiler and linker, this handle the linker
set(CMAKE_EXE_LINKER_FLAGS -mmcu=${MCU})

add_compile_options(
        -mmcu=${MCU} # MCU
        -std=gnu99 # C99 standard
        -Os # optimize
        -Wall # enable warnings
        -Wno-main
        -Wundef
        -pedantic
        -Wstrict-prototypes
        #        -Werror
        -Wfatal-errors
        -Wl,--relax,--gc-sections
        -g
        -gdwarf-2
        -funsigned-char # a few optimizations
        -funsigned-bitfields
        -fpack-struct
        -fshort-enums
        -ffunction-sections
        -fdata-sections
        -fno-split-wide-types
        -fno-tree-scev-cprop
)

# add compile definitions

file(GLOB SRC_FILES "src/*.c") # Load all files in src folder

add_executable(${PRODUCT_NAME} ${SRC_FILES})

# Set the output name of the target to be "name".elf (otherwise it just creates a file with output "name")
set_target_properties(${PRODUCT_NAME} PROPERTIES OUTPUT_NAME ${PRODUCT_NAME}.elf)

# add a custom target that transforms the binary (.elf) into a hex file (.hex)
# ALL -> indicates that target should be added to default build target (i.e. when make all called)
# DEPENDS -- target strip will be called first?
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex ${PRODUCT_NAME}.elf ${PRODUCT_NAME}.hex DEPENDS strip)

## Strip binary for upload -- avr-strip discards symbols from object files
add_custom_target(strip ALL avr-strip ${PRODUCT_NAME}.elf DEPENDS ${PRODUCT_NAME})

# Add target that uploads hex file to chip
#add_custom_target(upload avrdude  -c ${PROG_TYPE} -p ${MCU} -U flash:w:${PRODUCT_NAME}.hex DEPENDS hex)
add_custom_target(upload avrdude  -c ${PROG_TYPE} -p m328p -P ${PORT} -U flash:w:${PRODUCT_NAME}.hex DEPENDS hex)

# add source files for main

# Indicate that .hex files, .lst files can also be cleaned
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PRODUCT_NAME}.hex;${PRODUCT_NAME}.eeprom;${PRODUCT_NAME}.lst")

