# Hiding a KLD


When a KLD is loaded, it&rsquo;s presence is recorded on *2* lists: the linker_files and the modules lists. To hide our rootkit from kernel, we therefore need to hide it from these two lists.

We&rsquo;re going to be doing this through DKOM &ndash; Direct Kernel Object Maniulation (coined by Hoglund and Butler.) That is, we are abusing the fact that the operating system is storing internal record-keeping data in main memory by directly manipulating this data. There is no additional need for system call hooks.

There is an issue with this though, and it has to do with *synchronization*. Imagine we simply traverse some of the kernel&rsquo;s queue data structures and modify the ones we need to. If another thread tries to access or manipulate the same object at the same time, then we risk corrupting data (this is not something we really want at a kernel level, as I&rsquo;m sure you can imagine.)

To solve this issue, we use a *lock*. As described a bit more below, this will essentially control how multiple threads/cores can access a data object. For our purposes here, we will be using a *Giant* lock.

## Linker_files List

This is, unsurprisingly, the list of linker file structures mentioned above.

We have two locks we need to worry about about here: kld_mtx and Giant. The book says that kld_mtx doesn&rsquo;t properly protect linker_files, and to be totally honest I&rsquo;m just going to take its word for it here. I usually like to research it all nicely myself, but there&rsquo;s already so much to get through I&rsquo;m just going to leave it here. Anyway, because kld_mtx doesn&rsquo;t properly protect it, we also use giant.

What is a Giant lock? From Wikipedia: &ldquo;It is a solitary global lock that is held whenever a thread enters kernel space and released when the thread returns to user space.&rdquo; E.G. A system call. Because of this, &ldquo;threads in user space can run concurrently on any available processors, but no more than one thread can run in kernel space&semi; any other threads that try to enter kernel space are forced to wait. In other words, the giant lock eliminates all concurrency in kernel space.&rdquo;

The linker files structs themselves are held in a doubly linked tail queue:

From UNIX manual:

*   Tail queues add the following functionality:

    *   Entries can be added at the end of a list.
    *   They may be traversed backwards, from tail to head.
    *   They may be concatenated.
*   However:

    *   All list insertions and removals must specify the head of the list.
    *   Each head entry requires two pointers rather than one.
*   Also see:
    *   [https://man7.org/linux/man-pages/man3/tailq.3.html](https://man7.org/linux/man-pages/man3/tailq.3.html)

There is a counter that keeps track of how many files are in the list, so that when a new one is added it can be assigned a unique ID number.

Each linker file struct in the list is described in &lt;sys/linker.h&gt;. There are two main fields in the struct we care about: int refs &ndash; the number of inbound references to this struct&semi; and char *filename &ndash; the name of this linker file (i.e. our module&rsquo;s name when we see it by running kldstat.)

Normal 0 false false false EN-US X-NONE AR-SA

# Modules List

This is, like above, a doubly linked tail queue of module structs.

Like above, there is also a counter associated with this list (nextid).

The lock we&rsquo;re concerned about here comes from &lt;sys/module.h&gt; and is the struct sx. Unlike linker_files, this is a dedicated lock &ndash; *sx_xlock*. This is an exclusive lock. This means that *when our thread holds this lock, no other thread may hold it.* I.E: it will run exclusively.

As far as individual module structs go, the module is not defined in any header file &ndash; therefore we need to make a forward declaration of it in our code based off /sys/kern/kern_module.c. This will let us actually use and modify its values in our hiding function. The property we care about here is just char *name.