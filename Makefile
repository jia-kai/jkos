# $File: Makefile
# $Date: Fri Dec 03 15:45:31 2010 +0800

#
# This file is part of JKOS
# 
# Copyright (C) <2010>  Jiakai <jia.kai66@gmail.com>
# 
# JKOS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# JKOS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with JKOS.  If not, see <http://www.gnu.org/licenses/>.
# 

CXXSOURCES = $(shell find . -name "*.cpp")
ASMSOURCES = $(shell find . -name "*.s")
OBJS = $(patsubst %.cpp,obj/%.o,$(CXXSOURCES)) $(patsubst %.s,obj/%.o,$(ASMSOURCES))

CXX = g++
AS = as

INCLUDE_DIR = -I include
DEFINES = -DDEBUG
CXXFLAGS = -Wall -Wextra -Werror -Woverloaded-virtual -Wsign-promo -Wignored-qualifiers -Wfloat-equal -Wshadow \
		   -Wcast-qual  -Wcast-align -Wconversion  -Wlogical-op -Waggregate-return \
		   -Wmissing-noreturn  -Wpadded  -Winline -Woverlength-strings \
		   -nostdlib -nostartfiles -nodefaultlibs -fno-exceptions -fno-builtin -fno-rtti \
		   -fno-stack-protector $(INCLUDE_DIR) $(DEFINES) -g
OBJ_DIR = obj

hda.img: kernel.bin
	sudo losetup -o32256 /dev/loop0 hda.img
	sudo mount /dev/loop0 root
	sudo cp -av kernel.bin root
	sudo umount root
	sudo losetup -d /dev/loop0

Makefile.dep: $(CXXSOURCES) $(ASMSOURCES)
	for i in $(CXXSOURCES); do \
	$(CXX) $(INCLUDE_DIR) $(DEFINES) -M -MT "$(OBJ_DIR)/`echo $$i | sed -e 's/cpp\$$/o/g'`" $$i; \
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

.PHONY: qemu qemu-dbg clean hg
qemu: hda.img
	qemu -hda hda.img

qemu-dbg: kernel.bin
	qemu --kernel kernel.bin -S -s 

clean:
	rm -f kernel.bin $(OBJS)

hg:
	hg addremove
	hg commit -u jiakai
	hg push

