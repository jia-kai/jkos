/*
 * $File: kernel.cpp
 * $Date: Fri Nov 26 10:00:00 2010 +0800
 */
#include <multiboot.h>

struct Test
{
	Test(char ch)
	{
		unsigned char *videoram = (unsigned char *) 0xb8000;
		videoram[0] = ch; /* character 'A' */
		videoram[1] = 0x07; /* forground, background color. */
	}
};

Test t('B');

extern "C" void kmain(Multiboot_header_t* , unsigned int magic)
{
   if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
   {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
   }
 
   /* You could either use multiboot.h */
   /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
   /* or do your offsets yourself. The following is merely an example. */ 
   //char * boot_loader_name =(char*) ((long*)mbd)[16];
 
   /* Print a letter to screen to see everything is working: */
 
   /* Write your kernel here. */
}

