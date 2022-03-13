# Writing Function Hooking

So following along with the notes I made and the example given in the book, I was having a pretty easy time with this part. The code all made sense, and it seemed to fit in together.

Until it came to linking the original function back into the kernel when I unloaded my rootkit.

See, I was having huge issues finding read, mkdir, etc (as in, the actual function names of the original system calls). That is to say, when I was trying to undo my overriding of the system functions, I was having issues figuring out how to link them back during unload. This was the solution:

*   So read, mkdir, etc are now no longer the name of the system calls. Instead, this is calling the C standard library functions, which can only be run in the userspace (not kernel).
*   What we need is now sys_(name). See this patch note inside FreeBSD's changelog:

> r225617 | kmacy | 2011-09-16 06:58:51 -0700 (Fri, 16 Sep 2011) | 12 lines
> 
>  
> 
> In order to maximize the re-usability of kernel code in user space this patch modifies makesyscalls.sh to prefix all of the non-compatibility calls (e.g. not linux_, freebsd32_) with sys_ and updates the kernel entry points and all places in the code that use them. It also fixes an additional name space collision between the kernel function psignal and the libc function of the same name by renaming the kernel psignal kern_psignal(). By introducing this change now we will ease future MFCs that change syscalls.

And this fixed it! Writing `sysent[SYS_read].sy_call = (sy_call_t *)sys_read` instead of `sysent[SYS_read].sy_call = (sy_call_t *)read` now allowed the program to compile.

Onto the function hooking itself:

*   See this github link for the code (I'll also put it below): [https://github.com/callum-jones19/COMP6841_SA/blob/master/src/basic_function_hooking.c](https://github.com/callum-jones19/COMP6841_SA/blob/master/src/basic_function_hooking.c)
*   Right now, it simply overrides read() with a dummy print function. This is great because IT WORKS. It&rsquo;s not so great in that I essentially nuked read(), so when I run the rootkit the entire OS gets bricked. Very nice.
*   Interestingly, it seems to print my debug statement on a loop. There must be some function continuously reading from some source, so it could be interesting to look more into this later.
*   ![](https://www.openlearning.com/u/callumjones/blog/SomethingAwesomeFunctionHookingReflections/Picadfssgdture1.png?action=download)

*   As we can see, the function hook works! (In a manner of speaking). My guess is getty or something is just trying to read constantly (we can see it enters some process and never leaves).
*   I&rsquo;m calling this a victory today. Next session, I&rsquo;m going to clean up the actual content of the hook so it works as intended (hiding some pretend virus files from a user), and then on to hiding the actual rootkit from user detection.

Also, at this point I want to quickly acknowledge the fact that my set of Syscall notes from last week were actually extremely useful. I said back then that I wouldn't be using the content from those notes directly, but here I actually ended up needing to understand how syscall tables worked, etc, to be able to set up the function hook. So, to that extent, I'm glad I did extra research in the past -  it really came back to help me understand things a lot faster now.

## Function Hooking Code:
```C
#include <sys/types.h>;
#include <sys/param.h>;
#include <sys/proc.h>;
#include <sys/module.h>;
#include <sys/sysent.h>;
#include <sys/kernel.h>;
#include <sys/systm.h>;
#include <sys/syscall.h>;
#include <sys/sysproto.h>;

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
```
