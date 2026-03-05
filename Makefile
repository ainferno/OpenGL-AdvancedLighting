TARGET = $(basename $(notdir $(src)))

all:
	cmake -B build -G "MinGW Makefiles" -DSRC_FILE=$(src) && cmake --build build

run: all
	@build/$(TARGET).exe
