#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "userprog/process.h"
#include "threads/vaddr.h"


// Checks, if the user address is valid (by making sure, it's below PHYS_BASE and it's already mapped/allocated)
static int user_address_valid(void *addr) {
	if (addr == NULL || (!is_user_vaddr((uint32_t*)addr))) return false;
	else return (pagedir_get_page(thread_current()->pagedir, (uint32_t *)addr) != NULL);
}

// Checks, if the given number of pointers in a row are valid (returns 0 even if one of them is not)
static int pointers_valid(const void *address, uint32_t count) {
	const char *addr = (const char*)address;
	uint32_t i;
	for (i = 0; i < count; i++)
		if (!user_address_valid((void*)(addr + i))) return 0;
	return 1;
}

// Checks, if the given string is valid, by moving along it and verifying every single byte, stopping at '\0'.
static bool string_valid(const char *address) {
	while (true) {
		if (!user_address_valid((void*)address)) return false;
		if ((*address) == '\0') return true;
		address++;
	}
}


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}
