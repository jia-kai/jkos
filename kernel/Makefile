CXXSOURCES = $(wildcard *.cpp)
ASMSOURCES = $(wildcard *.s)
OBJS = $(patsubst %.cpp,obj/%.o,$(CXXSOURCES)) $(patsubst %.s,obj/%.o,$(ASMSOURCES))

CXX = g++
AS = as

INCLUDE_DIR = -I include
CXXFLAGS = -Wall -Wextra -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti \
		   -fno-stack-protector $(INCLUDE_DIR)
OBJ_DIR = obj

hda.img: kernel.bin
	sudo losetup -o32256 /dev/loop0 hda.img
	sudo mount /dev/loop0 root
	sudo cp -av kernel.bin root
	sudo umount root
	sudo losetup -d /dev/loop0

Makefile.dep: $(CXXSOURCES) $(ASMSOURCES)
	for i in $(CXXSOURCES); do \
	$(CXX) $(INCLUDE_DIR) -M -MT "$(OBJ_DIR)/`echo $$i | sed -e 's/cpp\$$/o/g'`" $$i; \
	done > Makefile.dep
	for i in $(ASMSOURCES); do \
	echo "$(OBJ_DIR)/`echo $$i | sed -e 's/s\$$/o/g'`: $$i" >> Makefile.dep; \
	done

sinclude Makefile.dep

obj/%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

obj/%.o: %.s
	$(AS) $< -o $@

kernel.bin: linker.ld $(OBJS)
	ld -T linker.ld -o kernel.bin $(OBJS)

.PHONY: qemu clean
qemu: hda.img
	qemu -hda hda.img

clean:
	rm kernel.bin $(OBJS)

