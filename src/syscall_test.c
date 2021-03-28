
/*
Example taken from "Designing BSD Rootkits." This file is not intended
to be passed off as my own work, but rather a practical implementation
following along with the example they give, so that I better understand
the notes I have written. I'll have a message to this effect at the
start of any files where the majority of code is not my own design.
*/

#include <sys/types.h>
#include <sys/sysproto.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>

/*
System call arguments
*/
struct sys_call_example_args {

	int tmp;

};

/*
System call function
*/
static int sys_call_example(struct thread *t, void *syscall_args) {

	struct sys_call_example_args *uap;
	// We typecast the arguments that the system passes into the
	// function to our own argument struct. This means in this case
	// that we are expecting a single string argument to this syscall.
	uap = (struct sys_call_example_args *)syscall_args;

	printf("%d\n", uap->tmp);

	return 0;
}

/*
SYSENT entry for the new system call
*/
static struct sysent sc_example_sysent = {
	1,			// Number of args
	sys_call_example	// Implementing function
};

// Offset variable.
static int offset = NO_SYSCALL;

/*
Event handler function
*/
static int load(struct module *module, int cmd, void *arg) {

	int err = 0;

	switch (cmd) {
	case MOD_LOAD:
		uprintf("System call loaded at offset %d.\n", offset);
		break;
	case MOD_UNLOAD:
		uprintf("System call unloaded from offset %d.\n", offset);
		break;
	default:
		err = EOPNOTSUPP;
		break;
	}

	return err;
}

SYSCALL_MODULE(syscall_test, &offset, &sc_example_sysent, load, NULL);
