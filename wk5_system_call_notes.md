# System Call Modules

A system call is the mechanism a program uses to request service from the kernel. E.G:

*   We have a program that reads data from one file and copies it into another file.
*   The information this program needs: data* from the user* with the *names* of the two files (input, output).
*   This flow will require the following system calls:

    *   Write a message (the prompt) to the screen.
    *   Read from an input to the OS (from the keyboard hardware.)

See [htt<span>ps://www.guru99.com/images/1/121119_0451_SystemCalli1.png</span>](https://www.guru99.com/images/1/121119_0451_SystemCalli1.png)<span> for a nice example image.</span>

**When we write our own system call, what we are essentially doing is writing a kernel module that can be called from userland code.**

## Requisite Functions/Data Types

* * *

**The System Call function:**

*   This is the actual function that implements the system call.
*   It takes its arguments from user space, but the function itself will execute in kernel space. Note that this means arguments must be passed in by value (not reference).
*   `typedef int sy_call_t(struct thread *, void *);`

    *   This is the prototype of the function.
    *   It&rsquo;s a return type int, and expects to be able to take in a thread pointer arg, and a void pointer arg.
    *   *NB because I forgot this: *(From GeeksForGeeks) A void pointer is a pointer that has no associated data type with it. A void pointer can hold address of any type and can be typecast to any type.
*   The system call args should be declared within their own struct (e.g., `struct sc_example_args { ... };`).

**Sysent Struct:**

*   An overview of the system call is defined by an entry in a sysent struct:

    ```C
    struct sysent {
            int sy_narg;            /* number of arguments */
            sy_call_t *sy_call;     /* implementing function */
            au_event_t sy_auevent;  /* audit event associated with system call */
    };
    ```

*   I say this is a sort of overview because an array of these structs exists in the system call table.
*   When a system call is installed to the kernel, this sysent struct is placed within a free slot in sysent[].

**Offset Value:**

*   This is a distinct integer between 0 &ndash; 456 that is assigned to each system call.
*   It indicates the sysent struct&rsquo;s offset in the sysent[] system call table.
*   In a system call module, this must be explicitly declared. This can be done with:
    * `static int offset = NO_SYSCALL;`. NO_SYSCALL sets offset to the next available free index in sysent[ ].

**SYSCALL_MODULE Macro:**

*   From the docs:

    *   `SYSCALL_MODULE(name, int *offset, struct sysent *new_sysent, modeventhand_t evh, void *arg);`
    *   [https://www.freebsd.org/cgi/man.cgi?query=SYSCALL_MODULE&sektion=9](https://www.freebsd.org/cgi/man.cgi?query=SYSCALL_MODULE&sektion=9)
    *   We could load our own syscall module into the kernel using `DECALRE_MODULE` (it is still a module after all). However, this would become pretty clunky &ndash; we&rsquo;d need to shove all the extra data SYSCALL_MODULE requires into another struct, which would then contain `evh, arg, offset, new_sysent, { 0 , NULL}`. See below for an example from Designing BSD Rootkits:
        ```C
            #define SYSCALL_MODULE(name, offset, new_sysent, evh, arg)     

            static struct syscall_module_data name##_syscall_mod = {       
                evh, arg, offset, new_sysent, { 0, NULL }               
            };                                                             

            static moduledata_t name##_mod = {                             
                #name,                                                  
                syscall_module_handler,                                 
                &name##_syscall_mod                                 
            };

            DECLARE_MODULE(name, name##_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE)
        ```

*   The highlighted line is where the extra args are passed into DECLARE_MODULE (recall that when we last used DECLARE_MODULE, we just left this blank) that are needed for a successful SYSCALL.
*   **NB:** ## is a pre-processor directive. Whenever something within that define has name##, it will be replaced by whatever we typed into the name var in SYSCALL_MODULE. This is why we don&rsquo;t need to put the name as a string.

For the Syscall Macro, the new arguments mean the following:

*   Offset &ndash; as above.
*   New_sysent &ndash; as above, passed as a struct sysent pointer.
*   Evh &ndash; event hander function.
*   Arg &ndash; for our purposes, this will be NULL.

## Executing the System Call

* * *

We need a couple of things first:

*   Modfind function

    *   `int modfind(const char *modname);`
    *   Returns a modid, which is used to identify each module in the system.
*   Modstat function:

    *   Returns the status of a kernel module from its modid.
    *   `int modstat(int modid, struct module_stat *stat);`
    *   The returned info is stored in the stat module_stat structure we passed in by reference.
*   Module_stat struct:
    ```
    struct module_stat {
            int             version;
            char            name[MAXMODNAME];       /* module name */
            int             refs;                   /* number of references */
            int             id;                     /* module id number */
            modspecific_t   data;                   /* module specific data */
    };
    typedef union modspecific {
            int             intval;                 /* offset value */
            u_int           uintval;
            long            longval;
            u_long          ulongval;
    } modspecific_t;
    ```
Syscall function:

*   Executes the system call specified by its system call number, which can be found above with modstat(it&rsquo;s the intval var).
*   `int systemcall(int number, ...)`

    *   The three dots after the first argument makes this function a variadic function. This means that it will accept an arbitrary number of parameters after the one that has already been defined.