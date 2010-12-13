# $File: Makefile
# $Date: Mon Dec 13 15:51:57 2010 +0800

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

CXXSOURCES = $(shell find src -name "*.cpp")
ASMSOURCES = $(shell find src -name "*.S")
OBJ_DIR = obj
OBJS = $(addprefix $(OBJ_DIR)/,$(CXXSOURCES:.cpp=.o) $(ASMSOURCES:.S=.o))
DEPFILES = $(OBJS:.o=.d)

CXX = g++
CC = gcc

INCLUDE_DIR = -I src/include
DEFINES = -DDEBUG
CXXFLAGS = -Wall -Wextra -Werror -Woverloaded-virtual -Wsign-promo -Wignored-qualifiers -Wfloat-equal -Wshadow \
		   -Wcast-qual  -Wcast-align -Wconversion  -Wlogical-op -Waggregate-return -Winline -Woverlength-strings \
		   -nostdlib -nostartfiles -nodefaultlibs -fno-exceptions -fno-builtin -fno-rtti \
		   -fno-stack-protector $(INCLUDE_DIR) $(DEFINES) -g

hda.img: kernel.bin
	sudo losetup -o32256 /dev/loop0 hda.img
	sudo mount /dev/loop0 root
	sudo cp -av kernel.bin root
	sudo umount root
	sudo losetup -d /dev/loop0

$(OBJ_DIR)/%.d: %.cpp
	$(CXX) $(INCLUDE_DIR) $(DEFINES) -M -MT "$(OBJ_DIR)/$(<:.cpp=.o) $(OBJ_DIR)/$(<:.cpp=.d)" "$<"  > "$@"

$(OBJ_DIR)/%.d: %.S
	echo "$(OBJ_DIR)/$(<:.S=.o): $<" > "$@"

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/%.o: %.S
	$(CC) $(INCLUDE_DIR) $(DEFINES) -c $< -o $@

sinclude $(DEPFILES)

kernel.bin: linker.ld $(OBJS)
	ld -T linker.ld -o kernel.bin $(OBJS)

.PHONY: qemu qemu-dbg clean hg
qemu: hda.img
	qemu -hda hda.img

qemu-dbg: kernel.bin
	qemu --kernel kernel.bin -S -s 

clean:
	rm -rf kernel.bin
	find obj -type f -delete

hg:
	hg addremove
	hg commit -u jiakai
	hg push

