PROJECT_SOURCEFILES +=
#maybe add file if we have more than one .c file

SMAC_PROJECT = blink

TARGET = cc2530dk
#no space after dk
# makefile.target not needed

all: $(SMAC_PROJECT)



test:
	$(warning $(SMAC_SOURCEFILES))
	$(warning $(SMAC_OBJECTFILES))
	$(warning $(PROJECT_OBJECTFILES))

SMAC = ../..

include $(SMAC)/Makefile.include

include $(SMAC)/platform/$(TARGET)/Makefile.$(TARGET)

blink: $(SMAC_OBJECTFILES)
	mkdir -p obj_cc2530dk
	$(CC) $(CFLAGS) $^

%.rel: %.c
	mkdir -p obj_cc2530dk
	$(CC) $(CFLAGS) -c $<
