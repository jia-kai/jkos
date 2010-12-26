/*
 * $File: elf.cpp
 * $Date: Sun Dec 26 21:28:47 2010 +0800
 *
 * functions for loading ELFs
 */
/*
This file is part of JKOS

Copyright (C) <2010>  Jiakai <jia.kai66@gmail.com>

JKOS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JKOS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JKOS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <elf_base.h>
#include <elf.h>
#include <errno.h>
#include <page.h>

static bool is_header_valid(const Elf32_Ehdr &header);

int load_elf(Fs::Node_file *file)
{
	if (!file)
		ERROR_RETURN(ENOENT);
	Elf32_Ehdr elf_header;
	if (file->read(&elf_header, sizeof(elf_header)) != sizeof(elf_header))
		ERROR_RETURN(EIO);
	if (!is_header_valid(elf_header))
		ERROR_RETURN(ENOEXEC);

	bool load_done = false;
	for (uint32_t i = 0; i < elf_header.e_phnum; i ++)
	{
		if (file->seek(i * sizeof(Elf32_Phdr) + elf_header.e_phoff, Fs::SEEK_SET) == (Fs::off_t)-1)
			ERROR_RETURN(EIO);
		Elf32_Phdr header;
		if (file->read(&header, sizeof(header)) != sizeof(header))
			ERROR_RETURN(EIO);

		if (header.p_type != PT_LOAD || !header.p_memsz)
			continue;

	}
	if (!load_done)
		ERROR_RETURN(ENOEXEC);
	panic("ok");
}

bool is_header_valid(const Elf32_Ehdr &header)
{
#define CHECK_EI(_index_, _val_) \
	do \
	{ \
		if (header.e_ident[_index_] != (_val_)) \
		return false; \
	} while (0)

	CHECK_EI(EI_MAG0, ELFMAG0);
	CHECK_EI(EI_MAG1, ELFMAG1);
	CHECK_EI(EI_MAG2, ELFMAG2);
	CHECK_EI(EI_MAG3, ELFMAG3);

	CHECK_EI(EI_CLASS, ELFCLASS32);
	CHECK_EI(EI_DATA, ELFDATA2LSB);
	CHECK_EI(EI_VERSION, EV_CURRENT);

#undef CHECK_EI
	return
		header.e_type == ET_EXEC && header.e_machine == EM_386 &&
		header.e_version == EV_CURRENT && header.e_entry &&
		header.e_phoff && header.e_phnum &&
		header.e_phentsize == sizeof(Elf32_Phdr);
}

