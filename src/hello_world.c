//Largely taken from book Designing FreeBSD Rootkits.

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>

static int load (struct module *module, int cmd, void *arg) {
    int err = 0;

    if (cmd == MOD_LOAD) {
        printf("Hello world!\n");
    } else if (cmd == MOD_UNLOAD) {
        printf("Goodbye, cruel world!\n");
    } else {
        err = EOPNOTSUPP;
    }

    return(err);
}

static moduledata_t hello_world_mod = {
    "hello",
    load,
    NULL
};

DECLARE_MODULE (hello, hello_world_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
