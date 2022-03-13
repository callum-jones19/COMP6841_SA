# My Implementation

Here we&rsquo;re basically just trying to implement and understand the system call module that is written in the FreeBSD Rootkit book here by writing one of our own (even though we don't really need a syscall module for our final rootkit.) I don't really intend for it to differ that much from the book's code, but I do intend to learn from what I'm writing

## Syscall module



So, I started by copying most of the stuff from the example given in FreeBSD book (I cut out some of the comments I had so it would fit). This is the original code, errors, included, so I can run through what went wrong and how I fixed it:
```C
struct sys_call_example_args {
        int tmp;

};
static int sys_call_example(struct thread *td, void *syscall_args)
{
        struct sc_example_args *uap;
        uap = (struct sc_example_args *)syscall_args;

        printf("%s\n", uap-&gt;str);
}

/* The sysent for the new system call. */
static struct sysent sc_example_sysent = {
        1,                      /* number of arguments */
        sc_example              /* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated. */
static int offset = NO_SYSCALL;

static int load(struct module *module, int cmd, void *arg)
{
        int error = 0;

        switch (cmd) {
        case MOD_LOAD:
                uprintf("System call loaded at offset %d.\n", offset);
                break;

        case MOD_UNLOAD:
                uprintf("System call unloaded from offset %d.\n", offset);
                break;

        default:
                error = EOPNOTSUP;
                break;
        }

        return(error);
}

SYSCALL_MODULE(sc_example, &offset, &sc_example_sysent, load, NULL);
```
When I compiled with make, I got the following errors:

![](https://www.openlearning.com/u/callumjones/blog/SomethingAwesomeSystemCallReflection/Pictuasdgre1.png?action=download)

For each error:

1.  Easy fix - I had changed the argument in my function to int (different to example code), but had forgotten to change the print statement inside the function.
2.  Again, easy fix - I just wasn't returning anything from the sys_call_example function.
3.  No ; after my struct declaration.
4.  EOPNOTSUP should have been EOPNOTSUPP, as in sys/module.h
5.  This error made me want to shoot myself, and it was the big one. I'll go into it more right below.

Error 5:

*   So it turns out since the Rootkit book was written, there was an extra addition to the `#includes` you need to run SYSCALL_MODULE. This is `#include <sys/sysproto.h>`. This took me like an hour of searching, and then looking through the example code the OS provides in /sys/examples/... / .
*   Easy right? No, and if this wasn&rsquo;t going on open learning I would have some much stronger things to say about it. Anyway, I chucked this into my includes and it did fix the AUE_NULL error. However, it then gave me like a billion `uint32_t` has not been defined. After another half hour, I found placing `#include <sys/sysproto.h>` after `#include <sys/param.h>` instead of before it fixed it.
*   This was too hacky for me, so I found out eventually that this file simply requires `#include <sys/types.h>` before it.

    *   But Callum, surely they would just include this in the sys/sysproto.h if it's a dependency?
    *   You know what&rsquo;s very dumb? This:

        ```
        #ifndef LOCORE
        #include <sys/types.h>;
        #endif
        ```

*   This is in the header file for `sysproto.h`. So it's obviously a dependency. But guess what? For god only knows what reason, IT DOESN&rsquo;T RUN. I do not have nearly enough energy to trace this problem to its core, so I'm just going to assume something changed somewhere else in the OS at some point, which cause this `#include` to not run.

Sick, so this finally compiles. But I ran it and it didn&rsquo;t print anything! Yay! This was a quick fix though. I was doing printf in my load function. However, printf in this context will just print to the kernel&rsquo;s buffer, which is only viewable with dmesg. Changing this to uprintf made it output the the userspace, and so I finally had this System call module working!

# Overall Reflections


Having finished this section, I actually don't know how relevant it is to the module I will be writing. I'm never actually going to be writing my own system call or load it directly into the system, so this might have been an unneccessary set of notes for the purpose of my rootkit. That said, I'm still glad I did it. I feel like I learnt a lot from how this process works.

Hope you enjoyed the stuff I learnt from this - it was a painful process.

 