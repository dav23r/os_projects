#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"


#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

struct lock filesys_lock;

// Locks the file system
void filesys_lock_acquire(void) {
	lock_acquire(&filesys_lock);
}
// Unlocks the file system
void filesys_lock_release(void) {
	lock_release(&filesys_lock);
}


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




/**
Terminates Pintos by calling shutdown_power_off() (declared in
‘devices/shutdown.h’). This should be seldom used, because you lose
some information about possible deadlock situations, etc.
*/
static void halt(void) {
	shutdown_power_off();
}

/**
Terminates the current user program, returning status to the kernel. If the process’s
parent waits for it (see below), this is the status that will be returned. Conventionally,
a status of 0 indicates success and nonzero values indicate errors.
*/
static void exit(int status) {
	thread_current()->exit_status = status;
	thread_exit();
}





/**
Runs the executable whose name is given in cmd line, passing any given arguments,
and returns the new process’s program id (pid). Must return pid -1, which otherwise
should not be a valid pid, if the program cannot load or run for any reason. Thus,
the parent process cannot return from the exec until it knows whether the child
process successfully loaded its executable. You must use appropriate synchronization
to ensure this.
*/
static pid_t exec(const char *cmd_line) {
	ASSERT(0);
}


/**
Waits for a child process pid and retrieves the child’s exit status.
If pid is still alive, waits until it terminates. Then, returns the status that pid passed
to exit. If pid did not call exit(), but was terminated by the kernel (e.g. killed due
to an exception), wait(pid) must return -1. It is perfectly legal for a parent process
to wait for child processes that have already terminated by the time the parent calls
wait, but the kernel must still allow the parent to retrieve its child’s exit status, or
learn that the child was terminated by the kernel.
wait must fail and return -1 immediately if any of the following conditions is true:
	• pid does not refer to a direct child of the calling process. pid is a direct child
	of the calling process if and only if the calling process received pid as a return
	value from a successful call to exec.
	Note that children are not inherited: if A spawns child B and B spawns child
	process C, then A cannot wait for C, even if B is dead. A call to wait(C) by
	process A must fail. Similarly, orphaned processes are not assigned to a new
	parent if their parent process exits before they do.
	• The process that calls wait has already called wait on pid. That is, a process
	may wait for any given child at most once.
Processes may spawn any number of children, wait for them in any order, and may
even exit without having waited for some or all of their children. Your design should
consider all the ways in which waits can occur. All of a process’s resources, including
its struct thread, must be freed whether its parent ever waits for it or not, and
regardless of whether the child exits before or after its parent.
You must ensure that Pintos does not terminate until the initial process exits.
The supplied Pintos code tries to do this by calling process_wait() (in
‘userprog/process.c’) from main() (in ‘threads/init.c’). We suggest that you
implement process_wait() according to the comment at the top of the function
and then implement the wait system call in terms of process_wait().
Implementing this system call requires considerably more work than any of the rest.
*/
static int wait(pid_t pid) {
	return process_wait(pid);
}


/**
Creates a new file called file initially initial size bytes in size. Returns true if successful,
false otherwise. Creating a new file does not open it: opening the new file is
a separate operation which would require a open system call.
*/
static bool create(const char *file, unsigned initial_size) {
	if (!string_valid(file)) exit(-1);
	else {
		lock_acquire(&filesys_lock);
		bool rv = filesys_create(file, initial_size);
		lock_release(&filesys_lock);
		return rv;
	}
	return false;
}


/**
Deletes the file called file. Returns true if successful, false otherwise. A file may be
removed regardless of whether it is open or closed, and removing an open file does
not close it. See [Removing an Open File], page 35, for details.
*/
static bool remove(const char *file) {
	if (!string_valid(file)) exit(-1);
	else {
		lock_acquire(&filesys_lock);
		bool rv = filesys_remove(file);
		lock_release(&filesys_lock);
		return rv;
	}
	return false;
}


/**
Opens the file called file. Returns a nonnegative integer handle called a “file descriptor”
(fd), or -1 if the file could not be opened.
File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is
standard input, fd 1 (STDOUT_FILENO) is standard output. The open system call will
never return either of these file descriptors, which are valid as system call arguments
only as explicitly described below.
Each process has an independent set of file descriptors. File descriptors are not
inherited by child processes.
When a single file is opened more than once, whether by a single process or different
processes, each open returns a new file descriptor. Different file descriptors for a single
file are closed independently in separate calls to close and they do not share a file
position.
*/
static int open(const char *file) {
	if (!string_valid(file)) exit(-1);
	else {
		lock_acquire(&filesys_lock);

		struct thread *this_thread = thread_current();
		
		file_descriptor fd = thread_get_free_fd(this_thread);
		if (fd >= 0) {
			struct file *opened_file = filesys_open(file);
			if (opened_file != NULL) {
				if (!thread_set_file(this_thread, opened_file, fd)) {
					file_close(opened_file);
					fd = -1;
				}
			}
			else {
				fd = -1;
			}
		}

		lock_release(&filesys_lock);
		
		return fd;
	}
	return -1;
}


/**
Returns the size, in bytes, of the file open as fd.
*/
static int filesize(int fd) {
	lock_acquire(&filesys_lock);
	struct file *file_ptr = thread_get_file(thread_current(), fd);
	int rv = ((file_ptr != NULL) ? file_length(file_ptr) : (-1));
	lock_release(&filesys_lock);
	return rv;
}


/**
Reads size bytes from the file open as fd into buffer. Returns the number of bytes
actually read (0 at end of file), or -1 if the file could not be read (due to a condition
other than end of file). Fd 0 reads from the keyboard using input_getc().
*/
static int read(int fd, void *buffer, unsigned size) {
	if (!pointers_valid(buffer, size)) exit(-1);
	else if (fd == STDIN_FILENO) {
		// Read from standard input
		unsigned int i;
		char *addr = buffer;
		for (i = 0; i < size; ++i)
			addr[i] = input_getc();
		return size;
	}
	else if (fd == STDOUT_FILENO) return 0;
	else {
		lock_acquire(&filesys_lock);
		struct file *file_ptr = thread_get_file(thread_current(), fd);
		int rv = ((file_ptr != NULL) ? file_read(file_ptr, (void*)buffer, size) : 0);
		lock_release(&filesys_lock);
		return rv;
	}
}


/**
Writes size bytes from buffer to the open file fd. Returns the number of bytes actually
written, which may be less than size if some bytes could not be written.
Writing past end-of-file would normally extend the file, but file growth is not implemented
by the basic file system. The expected behavior is to write as many bytes as
possible up to end-of-file and return the actual number written, or 0 if no bytes could
be written at all.
Fd 1 writes to the console. Your code to write to the console should write all of buffer
in one call to putbuf(), at least as long as size is not bigger than a few hundred
bytes. (It is reasonable to break up larger buffers.) Otherwise, lines of text output
by different processes may end up interleaved on the console, confusing both human
readers and our grading scripts.
*/
#define CHUNCK_SIZE 100  // 100 bytes per chunck
static int write(int fd, const void *buffer, unsigned size) {
	if (!pointers_valid(buffer, size)) exit(-1);
	else if (fd == STDOUT_FILENO) {
		// Write to standard output by chuncks of CHUCK_SIZE
		const char *addr = buffer;
		unsigned int rem_size = size;
		while (rem_size > 0) {
			unsigned int to_write = min(CHUNCK_SIZE, rem_size);
			putbuf(addr, to_write);
			rem_size -= to_write;
			addr += to_write;
		}
		return size;
	}
	else if (fd == STDIN_FILENO) return 0;
	else {
		lock_acquire(&filesys_lock);
		struct file *file_ptr = thread_get_file(thread_current(), fd);
		int rv = ((file_ptr != NULL) ? file_write(file_ptr, (void*)buffer, size) : 0);
		lock_release(&filesys_lock);
		return rv;
	}
}


/**
Changes the next byte to be read or written in open file fd to position, expressed in
bytes from the beginning of the file. (Thus, a position of 0 is the file’s start.)
A seek past the current end of a file is not an error. A later read obtains 0 bytes,
indicating end of file. A later write extends the file, filling any unwritten gap with
zeros. (However, in Pintos files have a fixed length until project 4 is complete, so
writes past end of file will return an error.) These semantics are implemented in the
file system and do not require any special effort in system call implementation.
*/
static void seek(int fd, unsigned position) {
	ASSERT(0);
}


/**
Returns the position of the next byte to be read or written in open file fd, expressed
in bytes from the beginning of the file.
*/
static unsigned tell(int fd) {
	ASSERT(0);
}


/**
Closes file descriptor fd. Exiting or terminating a process implicitly closes all its open
file descriptors, as if by calling this function for each one.
*/
static int close(int fd) {
	int rv = -1;
	struct thread *t = thread_current();
	struct file *fl = thread_get_file(t, fd);
	if (fl != NULL) {
		thread_set_file_force(t, NULL, fd); // File is closed automatically inside of this function.
		rv = 0;
	}
	return rv;
}





#define ESP ((uint32_t*)f->esp)
#define PARAM(id) (ESP + id)
#define I_PARAM(id) (*PARAM(id))
#define S_PARAM(id) ((char*)I_PARAM(id))
#define V_PARAM(id) ((void*)I_PARAM(id))
#define EAX (f->eax)

static int check_args(struct intr_frame *f, int start, int end) {
	while (start < end) {
		if (!pointers_valid(PARAM(start), sizeof(void*))) return 0;
		start++;
	}
	return 1;
}

static void halt_handler(struct intr_frame *f UNUSED) {
	halt();
}
static void exit_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else exit(I_PARAM(1));
}
static void exec_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = exec(S_PARAM(1));
}
static void wait_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = wait(I_PARAM(1));
}
static void create_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 3)) exit(-1);
	else EAX = create(S_PARAM(1), I_PARAM(2));
}
static void remove_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = remove(S_PARAM(1));
}
static void open_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = open(S_PARAM(1));
}
static void filesize_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = filesize(I_PARAM(1));
}
static void read_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 4)) exit(-1);
	else EAX = read(I_PARAM(1), V_PARAM(2), I_PARAM(3));
}
static void write_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 4)) exit(-1);
	else EAX = write(I_PARAM(1), V_PARAM(2), I_PARAM(3));
}
static void seek_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 3)) exit(-1);
	else seek(I_PARAM(1), I_PARAM(2));
}
static void tell_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = tell(I_PARAM(1));
}
static void close_handler(struct intr_frame *f) {
	if (!check_args(f, 1, 2)) exit(-1);
	else EAX = close(I_PARAM(1));
}

#define MAX_SYS_CALL_ID \
				max( \
					max( \
						max( \
							max(\
								max(SYS_HALT, SYS_EXIT), \
								max(SYS_EXEC, SYS_WAIT) \
							), \
							max( \
								max(SYS_CREATE, SYS_REMOVE), \
								max(SYS_OPEN, SYS_FILESIZE) \
							) \
						), \
						max( \
							max( \
								max(SYS_READ, SYS_WRITE), \
								max(SYS_SEEK, SYS_TELL) \
							), \
							max( \
								max(SYS_CLOSE, SYS_MMAP), \
								max(SYS_MUNMAP, SYS_CHDIR) \
							) \
						) \
					), \
					max( \
						max(SYS_MKDIR, SYS_READDIR), \
						max(SYS_ISDIR, SYS_INUMBER) \
					) \
				)

#define SYS_COUNT (MAX_SYS_CALL_ID + 1)
static const int sys_count = SYS_COUNT;
typedef void(*sys_handler)(struct intr_frame*);
static sys_handler sys_handlers[SYS_COUNT];
static bool sys_initialized = false;

static void init_sys_handlers(void) {
	if ((!sys_initialized) || (sys_handlers[SYS_HALT] != halt_handler)) {
		int i;
		for (i = 0; i < sys_count; i++)
			sys_handlers[i] = NULL;
		sys_handlers[SYS_HALT] = halt_handler;
		sys_handlers[SYS_EXIT] = exit_handler;
		sys_handlers[SYS_EXEC] = exec_handler;
		sys_handlers[SYS_WAIT] = wait_handler;
		sys_handlers[SYS_CREATE] = create_handler;
		sys_handlers[SYS_REMOVE] = remove_handler;
		sys_handlers[SYS_OPEN] = open_handler;
		sys_handlers[SYS_FILESIZE] = filesize_handler;
		sys_handlers[SYS_READ] = read_handler;
		sys_handlers[SYS_WRITE] = write_handler;
		sys_handlers[SYS_SEEK] = seek_handler;
		sys_handlers[SYS_TELL] = tell_handler;
		sys_handlers[SYS_CLOSE] = close_handler;
		sys_initialized = true;
	}
}


static void syscall_handler(struct intr_frame *);

void
syscall_init(void)
{
	intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
	init_sys_handlers();
	lock_init(&filesys_lock);
}

static void
syscall_handler(struct intr_frame *f UNUSED)
{
	if (!pointers_valid(ESP, sizeof(int))) exit(-1);
	init_sys_handlers();
	int syscall_id = *ESP;
	if (syscall_id >= 0 && syscall_id < sys_count && sys_handlers[syscall_id] != NULL) {
		sys_handlers[syscall_id](f);
	}
}
