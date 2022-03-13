# Completing Function Hooking/Adding Proper Functionality to the Rootkit

## Intro

* * *

So, to just briefly start &ndash; I know that this was written and posted in Week 8, but these are actually my reflections from Week 7. The reason I&rsquo;m only posting them now is that I want my write-ups to be really good quality, with clear and concise formatting (plus some of the stuff at the end was done in Week 8 - took me that long. In week 7, working on file hiding function hooking I spent so much time simply trying to get it to work and logging what I tried in rough bullet points that it would have taken me more time than I had in that week to actually write it up properly. That said, I want to show that I was taking notes and doing work consistently, so [**here**](https://docs.google.com/document/d/19N5m1hmuFRNeBHWm8AOi4m2g6kTgvTGGm1R5ZKwvCfQ/edit?usp=sharing) is a Google doc with the rough notes I wrote this post from (I censored the many swear words in it for professionalisms sake). [**Here **](https://docs.google.com/document/d/11Ok4D12r3P2EZwQjxWbbyMZ9HPVvXlwjoIv8tcBSIK8/edit?usp=sharing)is a time log from that weekend as well. This picture shows that this file was in fact made then:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture1.png?action=download)

**NB:**All terminal outputs are going to be included as pictures. I would have loved to dump the outputs to a file, but I had so much trouble trying to get the FreeBSD VM to copy to the host machine I just gave up. So, pictures it is.

**NB 2:** So, this is a long section. I want to point out at the start that there is a lot of trial and error in this document. You&rsquo;ve been warned. There are reflections on the entire process at the very end, if you&rsquo;d like to skip to those first.

<div>

## **<span>Finding the Processes that I Need to Hook</span>**

* * *
</div>

So, at this point in the process, I have a module that can hook into a system process, but I haven&rsquo;t chosen which system calls I want to hook, and I haven&rsquo;t added any actual functionality to the hooks. Let&rsquo;s work on finding which call to hook first.

### ***<span>Analysing &lsquo;ls&rsquo;</span>***

Recall that the purpose of this rootkit is to hide a file from the user (a fake virus binary, or something like that). So, it makes sense that we start by looking at **ls**. Using ktrace, we can record a log file of all the system calls made when this function is run (the full command being *ktrace ls*). Having generated this, we can then view it by simply running *kdump*. The problem with this is that there is a LOT of detail here, most of which I don&rsquo;t care about:

So, using my wonderful knowledge of UNIX I piped this output into *grep*, filtering by any lines which had CALL in them (because we just care about finding the relevant system calls *ls* uses.) The command itself was just: *kdump | grep &ldquo;CALL&rdquo;*. This gave a much easier-to-read output:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture2.png?action=download)

As you can see, there is still a lot of stuff here. To choose the relevant calls, we&rsquo;re just going to use a sort of guess-and-check reasonableness test. The first call that jumps out is *getdirentries()*. We&rsquo;ll start there.

To test this, what I&rsquo;m doing is just changing my onLoad module function to override the SYS_getdirentries call with my function. In my function, I&rsquo;m then not even going to call the original system function; instead, I'm just going to print a test statement to user space using `uprintf("Test\n")`. This will let me see when the hook is being called.

Having done this, and then loaded the module, this is the result:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture3.png?action=download)

And this is very promising! We can see that any functions (i.e. ls) which try to analyse the contents of a directory will be overridden, and TAB autocomplete no longer works either.

One problem I can see though is that mkdir doesn&rsquo;t have this system function called (see image.) This means it recognises when a folder exists there already with that name. This could be an issue, as then the user would know something&rsquo;s being hidden there. In this case, how can we cover this up?

### ***<span>Analysing &lsquo;mkdir&rsquo;</span>***

So, to take a look at what we might need to do to cover up mkdir, let&rsquo;s *ktrace *and *kdump* again.

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture5.png?action=download)

So *getdirentries* isn&rsquo;t there, but a system call called *stat()* is. Looking in the system calls chapter of the FreeBSD Man, we can see that *stat()* is:

    <q>The **stat**() system call obtains information about the file pointed to by path.<span>  </span>Read, write or execute permission of the named file is not re-quired, but all directories listed in the path name leading to the file must be searchable.</q>

This doesn&rsquo;t need getdirentries(), because it&rsquo;s just looking straight at a supplied path name. It&rsquo;s therefore probably a good idea to hook stat() as well.

## ***<span>Hooking the System Calls</span>***

* * *

I&rsquo;m going to write a blank hook for stat() and getdirentries() first. See the below code:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture6.png?action=download)

Recall from my function hooking 1 writeup that system call functions are now prefixed with *sys_[function name]*. This means my hooks now actually invoke the original call at some point, making them proper middle-man functions. It will run my code and also execute the original system call function (in whatever order I require.) Very slick.

I had issues with the hook for stat, using this code:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture7.png?action=download)

This didn&rsquo;t make sense, but then I realised that it&rsquo;s because I was loading the stat_hook function into the sys_read syscall table entry (i.e. I was overriding the wrong function.) Silly mistakes are everywhere.

With that fixed, the hooks were both working! Now onto overriding the actual function content.

Unfortunately, at this point I discovered a fatal kernel bug in my software. It was reproducable though &ndash; I tested it for a few minutes and discovered it would always happen when I loaded and then unloaded the module. It was never immediately after unloading, but anywhere from 3 to 8 seconds.

Looking at the unloading segment of my code, I realised it was because I forgot to hook up the kernel function back to the system call when I unloaded the module. I was basically NullPointerExceptioning my kernel (or I guess segfaulting, given that it&rsquo;s C). How awesome is that.

##  

## ***<span>The Painful Part &ndash; Figuring Out How/What Objects to Modify</span>***

* * *

Now I just need to figure out how to fetch the arguments of the hooked call, modify them and pass them back to ignore a specific file directory.

For stat(), this involved first looking through the man page:

* [https://www.freebsd.org/cgi/man.cgi?query=stat&apropos=0&sektion=2&manpath=FreeBSD+11.2-RELEASE&arch=default&format=html](https://www.freebsd.org/cgi/man.cgi?query=stat&apropos=0&sektion=2&manpath=FreeBSD+11.2-RELEASE&arch=default&format=html)

I thought this seemed promising &ndash; it had a files inode number, the references it had, its modification data, etc. I figured that this seemed like the sort of content we would want to modify, were we to want to hide something from a function like *mkdir* (in retrospect, it was dumb to start working on mkdir first &ndash; I might as well have started on the stuff I knew I needed to hook, but this is a lesson I learnt the hard way.)

To take a deeper look at what I might need to modify, I just started by printing out struct values I thought would be useful. In particular, st_dev and st_ino, which the man says &ldquo;together uniquely identify the file within the system.&rdquo;

However, after playing around with this information for nearly an hour, I still was none closer to figuring out what data I might need to change in the struct to hide the file from mkdir or similar functions. At this point, I *just accepted my losses and decided to move on* to hooking getdirentries, which I figured would be more directly related to what I was trying to achieve regardless. I figured that if I had time later, I could work on hiding a file from functions like mkdir().

I also at this point realised that stat() might not even be the part of mkdir that finds whether a file already exists. Mkdir is a system binary, so it could just be doing a search itself. As I said, at this point I moved on, so I actually still am not 100% on this, but I&rsquo;m pretty sure it&rsquo;s the case.

So, getdirentries(). I began by pulling up the man page, which gave the following function prototype: int getdirentries(int fd, char *buf, int nbytes, long *basep). It also told me that the files it found in a directory were stored in a list of struct dirents, whose properties are explained on its man page.

When we call sys_getdirentries(thread td, syscall_args), syscall_args would contain the actual arguments that the function as described in the prototype would be receiving. With this knowledge, I could analyse the data getdirentries was using through this argument struct (i.e. uap-&gt;fd, uap-&gt;basp, etc).

At this point, I still had a few issues and confusions:

* why is the sys_getdirentries parameters different to getdirentries() prototype in the man?

* why is buf giving me a seg fault when I try to print it as a string?

* why is nbytes apparently not an arg.

The first answer I figured out is the buf one. It&rsquo;s because I&rsquo;m an idiot and didn&rsquo;t realise it was a struct list of type struct dirent. I was thrown off by the fact that it was *char **. That was fairly simple.

The other two I was still confused about. On top of this, I felt like I had the info I needed, but didn&rsquo;t understand what to do with it. How did I modify buf, etc. I spent maybe an hour just messing around with the arguments to see how I could get the buffer to hide things, but with no success. It felt stagnated. That was, until I discovered&hellip; That this topic was A SECTION IN THE FREEBSD BOOK I WAS USING ALL ALONG. This:

A) Sucked, because it meant that I wouldn&rsquo;t be able to show I did all of this with no help.

B) Was great because I needed some guidance at this point.

With this, things started moving quickly again. The book pointed me into the right direction by showing me where the vfs_syscalls.c file was, which contained the source of the functions I was hooking. This answered the remaining questions I had. At this point, I had spent about 4 hours just looking around for stuff, and so I&rsquo;d like to take a moment to reflect that these sorts of projects often involve huge amounts of time where you feel like you&rsquo;ve achieved nothing, but it all builds to some revelation at the end.

Now that I had syscalls.c, I found the definition of sys_getdirentries. Two things of note:

* There was another function called kern_getdirentries which had the params listed in the man page. This explained my question earlier. Sys_getdirentries (thread , syscall args) was a wrapper around the actual kern function, hence the two difference arguments.

* The reason I was getting *nbytes is not an argument* earlier was because it wasn&rsquo;t. It was called *count* in the args. **Huge shoutout to the docs for being wrong**.

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture8.png?action=download)

* Finally, I discovered that in kern_getdire ntries, there is an important variable defined. While for most of the file struct list, info is stored in buf, there is an int called size which stores the length of the entire struct dirent list. This is actually returned to td-&gt;td_retval[0]. This isn&rsquo;t in the docs, just the source code. This is important because this needs to be modified if we want to properly hide the getdirentries return value. With this knowledge, I decided to stop for the night, and then return to properly complete getdirentries_hook armed with this new knowledge.

##  

<div>

## **<span>Tuesday Week 8 &ndash; Implementing </span>**

* * *
</div>

So now this is the work I did in Week 8 on this. I know. Took me a while didn&rsquo;t it <span>☹</span>.

With all of the above drama behind me, I was ready to get through this function hook.

Getting up the code of kern_getdirentries(), I started by reading through the whole function. There was a lot of irrelevant stuff, so I was just concerned with figuring out what would be important to me. In this case, that&rsquo;s mainly:

* Where it&rsquo;s saving its data, and what the pointers are (+ will we have access to them in our hook).

Looking through the rest of the function, we can see that:

* The buffer pointer is stored in the struct aiov.base. It turns out we don&rsquo;t care about this, but looking at that struct made me then look at auio, which is a struct used later. One of its values is relevant, as it is used in the line td-&gt;td_retval[0] = uap-&gt;count &ndash; auio.uio. Looking further at this line, uap-&gt;count is (from the getdirents man page) the max amount of data getdirentries can transfer. Looking at this code here, we see that auio.uio_resid is set to this value near the start of the function, but updated each time a file is found. **I.E**. If nothing is found in that directory, td-&gt;td_retval[0] will be 0. Thinking about this more, we can see that this value is the size of the list of dirent structs getdirentries() returns. Therefore, this is a critical piece of info we need to override in our hook.

* The only other thing we care about at this point is the buf. We need to modify that by finding the dirent struct that represents our hidden file and patch it out of the results.

At this point I had enough info to actually start implementing some of the code. The process I am following was documented on paper because I was having trouble visualising what the code would do &ndash; returning to good ol&rsquo; diagrams to work with memory in C.

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture9.jpg?action=download)

Typing up that checklist:

1) Check each dirent in getdirentries()&rsquo;s buf.

<span>                </span>a) To do this, we cast a struct dirent called curr to be the value of buf.

<span>                </span>b) then to iterate, we add the size of that struct to the pointer, which will move us to the next one in memory. We keep going until we either break when we find our target file, or until count==0, at which point we&rsquo;ve reached the end of the struct dirent list.

2) If the name matches our target file, we want to hide that entry.

3) As this is not an array or linked list (just sits in mem), we need to write over the memory to delete it (i.e. we can&rsquo;t just change a pointer to hide the entry, ect).

After my first rough attempt at writing the code, I had quite a few errors (I forgot to take a pic of the code at this stage, sorry.) See the pic below.

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture10.png?action=download)

It turns out this was just a missing import, though.

I was also getting trouble with simply accessing uap (which was the struct holding the arguments to getdirents, and so by extension the struct I was using to access buff) directly. While this was explained in the FreeBSD rootkit book, I tried to do as much as I could without using it to help (so I wasn&rsquo;t just blindly copying.) After researching it in the book, though, I discovered it was because this buffer of struct dirents sits in user space. This makes sense, as the user is the one who will eventually need them. However, kernel memory is different to userland memory, so to work with it we need to copy buff down to a local variable in kernel, work with that and then push it back up to user memory.

With this in mind, I rewrote some of the code and got it to this:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture11.png?action=download)

This is just printing out all the file in a directory whenever getdirentries is called. Useless, I know, but it showed me I was properly accessing the list of struct dirents. Cool so that all works.

Following this, I actually wrote the code to hide the file when its found. This was just a simple strcompare, and then if the dirent struct for our target file exists, we fetch everything one ahead of that struct in the list, copy it, and paste it over the top of the struct we&rsquo;re hiding (imagine picking everything infront of that entry in the list up, and just moving it back one so that it hides the target struct behind it.)

And this nearly worked &ndash; but there was a weird bug:

<div>

## **<span>The Weird Bug</span>**

* * *
</div>

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture12.png?action=download)

You can see (only) when it&rsquo;s hiding the not_a_virus file, exactly one of the other files&rsquo; name gets chopped. Bit suspicious, isn&rsquo;t it? I just need to fix that bug, and then it&rsquo;ll be done.

So, a couple of days later, I did some bug testing:

* It doesn&rsquo;t break when I comment out my code that actually hides the file. This means that the bug is somewhere in this chunk of code:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture13.png?action=download)

The character that replaced the file name was also seemingly random. Sometimes it was blank, sometimes an ampersand, sometimes a random char:

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture14.png?action=download)

![](https://www.openlearning.com/u/callumjones/blog/SaFunctionHookingWriteupAndReflection2/Picture15.png?action=download)

I tried a NULL TERMINATOR at the end of my #defined target file name, but that didn&rsquo;t fix it.

At this point, I wanted to try to directly inspect the code and relevant memory in real-time, so I started looking into kernel equivalents of GDB. After 20 or so minutes of searching, I found ddb. This needed some additional kernel configuration, which I did, and then the kernel had to be loaded into debug mode. After 40 minutes I still couldn&rsquo;t get this to work, so I gave up.

Then I tried printfs everywhere. I got a couple of things from this.

* The file that gets chopped is always the one after the virus file that&rsquo;s being hidden.

* All the code is running when it should be.

That first point led me to think it was something to do with the size of each struct. And it turned out to be right! After drawing a diagram similar to the one I posted above, I realised that the issue was as follows:

1. We&rsquo;re using bcopy() to overwrite the struct we are hiding with all future structs in the list. We do this by taking the pointer of the target to-hide struct, and then saying to the program &ldquo;copy all the content in the list from this address + this struct&rsquo;s size over this one.&rdquo;

2. We then subtract the length of that hidden struct from size (size being the variable that represents the total size of the list of structs), meaning that the function knows how big the total list is.

3. The problem was that as it was written, whenever I grabbed the size of the struct we were hiding, I just did curr-&gt;d_reclen (i.e. curr-&gt;this_structs_size.) The issue here, is that when we use bcopy, we are overwriting what&rsquo;s at the address of curr. That means, the length of the struct we are getting is not the length of the struct we have hidden, but the length of the struct we are overriding it with. As a result, the recorded size of the whole list ends up being incorrect, meaning the returned list gets corrupted in the weird way we were seeing above.


## Reflections


I&rsquo;m going to keep this brief because I realise how obnoxiously long this section is. I&rsquo;m sorry, there was a huge amount of work that went into it and I wanted to prove how much I did and the process I followed.

The first reflection is that working on something like this with no help really sucks. As you can tell from above, I had no real guidance up until quite late into the process. Here&rsquo;s what I took away from that:

* It&rsquo;s very, very slow and frustrating when you&rsquo;re working from the ground up. I think there&rsquo;s a certain level of acceptance you need, in recognising that you&rsquo;re going to try a lot of things that don&rsquo;t work. I think this was an important take away though, because at a certain point in my professional life, I&rsquo;m sure there are going to be plenty of instances where I am actually trying something that no one else knows how to do either. Learning that the frustration of not seeing tangible results for ages is a necessary part of this kind of discovery is an important lesson.

* Where you don&rsquo;t need to, don&rsquo;t try to always reinvent the wheel. If someone has idea on how to approach something, its useful to read them even if you don&rsquo;t want help. Seeing another approach to a problem can help you to adapt and learn.

The second reflection is on the stuff I researched which ended up not being useful for this part:

* I realise now that with the stuff I learnt over the whole process, I can now look back at stat&rsquo;s man page and have a much clearer idea how I&rsquo;d attack the function. It wasn&rsquo;t a loss me spending time on it, because its knowledge I might find useful in a future endeavour.

* Don&rsquo;t be stubborn &ndash; I knew getdirentries was the most important file to look at, but I didn&rsquo;t start with it.

Overall, though, I&rsquo;m beyond proud of this section. I had no prior experience working with an OS (save for 1521, which was ages ago and didn&rsquo;t look at this sort of stuff), but I learnt so much and have such a better understanding of how all these systems work (and why they can be vulnerable). Thanks for sticking through this trek of a post, if you&rsquo;re still here.

## Code

* * *
```C
static int dir_hook (struct thread *td, void *syscall_args) {

    struct getdirentries_args *uap;
    uap = (struct getdirentries_args *)syscall_args;

    // We have the old and modified return values:
    struct dirent *curr, *dp;
    // We set up size to match the variable in uap-&gt;count (in the original file)
    u_int size, count;

    // Run getdirents and store the data into uap-&gt;buf.
    int err = sys_getdirentries(td, uap);
    // This is the actual amount of bytes in the fd.
    size = td-&gt;td_retval[0];

    // If this file directory actually contains any information:
    if (size &gt; 0) {
        // Copy the buffer into kernel memory so we can actually use it.
        MALLOC(dp, struct dirent *, size, M_TEMP, M_NOWAIT);
        copyin(uap-&gt;buf, dp, size);

        //First direntry struct in the list.
        curr = dp;
        count = size;
        u_int len = 0;
        // Iterate through all directory entries here.
        while (count &gt; 0 && curr-&gt;d_reclen != 0) {
            // Reduce count by the size of this direntry.
            len = curr-&gt;d_reclen;
            count -= len;

            // uprintf("Current file: %s\n", curr-&gt;d_name);
            // uprintf("Current file has size: \n");

            // Do we want to hide the file?
            if (strncmp((char *)&curr-&gt;d_name, TARGET_NAME, LENGTH) == 0) {
 				// Only need to cut it out if it's in the
 				// middle of the list. If it's at the end
  				// it'll get cut by the smaller size.
                // uprintf("1\n");
                if (count != 0) {
                    // uprintf("2\n");
                    bcopy((char *)curr + len,
                            curr, count);
                    // uprintf("3\n");
                }

                size -= len;
                // uprintf("4\n");
                break;
            }
            // uprintf("5\n");

            // Update curr to point to the next struct if there
            // are more.
            if (count != 0) {
                curr = (struct dirent *) ((char *)curr + len);
            }
        }

        // uprintf("6\n");

        // After the loop: if we found something, then this
        // will adjust the return values that the user can see.
        td-&gt;td_retval[0] = size;
        copyout(dp, uap-&gt;buf, size);
        // uprintf("7\n");
        FREE(dp, M_TEMP);
        // uprintf("8\n");
    }

    return err;
}
```

 