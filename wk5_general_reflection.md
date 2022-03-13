# General Reflections
So I had this in MS Word and forgot to post it - oops.

Anyway, Week 5 was not very productive for my Something Awesome (see also Week 5 time management reflection). I was completely out of it during the week doing a law essay and over the weekend I basically slept the entire time to recover my energy. Hence nothing was really done on it this week.

I got the chance to read up on System Calls though, although just briefly. So far I have the following:

*   A system call is the mechanism a program uses to request service from the kernel.
    *   We have a program that reads data from one file and copies it into another file.
    *   The information this program needs: data* from the user* with the *names* of the two files (input, output).
    *   This flow will require the following system calls:
        *   Write a message (the prompt) to the screen.
        *   Read from an input to the OS (from the keyboard hardware.)
    *   [https://www.guru99.com/images/1/121119_0451_SystemCalli1.png](https://www.guru99.com/images/1/121119_0451_SystemCalli1.png)
    *   When we write our own system call, what we are essentially doing is writing a kernel module that can be called from userland code.

*   SYSCALL_MODULE Macro

    * `#define SYSCALL_MODULE(name, offset, new_sysent, evh, arg)`
        *   This is the declare macro.
    *   We could load our own syscall module into the kernel using DECALRE_MODULE (it is still a module after all). However, this would become pretty clunky &ndash; we&rsquo;d need to shove all the extra data SYSCALL_MODULE requires into another struct (we would call it something like [name]_syscall_mod), which would then contain `evh, arg, offset, new_sysent, { 0 , NULL}.

This is at least a good point to go with for Week 6. I at least understand how these system calls work, so now my learning's going to be focusing on practically loading them and then moving on to my Credit grade.

At this point, it's worth noting that my time plan for this project has completely gone out the window. On that note:

# My new plan:

Make my old week 6 deadline back to week 7 (which was free for flexibility time anyway). With that, have the week 5 task as a deadline for the end of next week - I'll post all the stuff I have at the end of the week.