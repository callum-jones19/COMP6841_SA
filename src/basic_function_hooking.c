#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>


// System call hook function
static int read_hook (struct thread *td, void *syscall_args) {

	// TODO
	uprintf("Teesstt!\n");

	return 0;
}

// Function called at load/unload
static int load (struct module *module, int cmd, void *arg) {

	int err = 0;

	switch(cmd) {
	case MOD_LOAD:
		uprintf("Loaded module\n");
		sysent[SYS_read].sy_call = (sy_call_t *)read_hook;
		break;
	case MOD_UNLOAD:
		sysent[SYS_read].sy_call = (sy_call_t *)sys_read;
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
