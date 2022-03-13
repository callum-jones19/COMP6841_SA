# Introduction

Since my last project idea brainstorm, I had to change my project idea. Instead of modifying it slightly, I decided that I would rather give myself a more technical challenge, and so decided to work on a rootkit for the FreeBSD 11.2 operating system.

This interests me for a few reasons:

1.  It&rsquo;s quite technical, and entirely software based. I always have found it more fun working entirely with software, and I really enjoy getting familiar with a subject that is, without knowing much about it, quite a daunting task.
2.  Rootkits form a large foundation of security threats. I&rsquo;d like to understand this better, and making one is one of the best ways to do this.

This would be highly relevant for the course. Rootkits are installed (normally) through a breach of trust (maybe a Trojan horse, etc), and involve all sorts of security issues such as invisible risk.

# The Plan

I have no familiarity with rootkits or how they function. Similarly, I haven&rsquo;t coded at a kernel level before, so I will need to get familiar with how such software would be written. Thankfully, I&rsquo;m confident at writing in C.

My plan so far is to read through the book *Designing BSD Rootkits*, which will go over most of the techniques/approaches I&rsquo;ll be able to use. The book&rsquo;s around 160 pages so I&rsquo;m not going to be using all of it, but I think as a rough guide it will help give me direction in how fundamental features of the rootkit are meant to work.

# Progress Updates

I plan to do one update at the end of each week detailing exactly what I did, what I learnt, and if what I have done has changed anything I am planning to do. These will hopefully act as a record of any troubles I ran into, and what solutions I found to them.

I will keep the project updated on a private GitHub repo as well, so that commits can be verified via timestamps (for portfolio time management). I&rsquo;ll also make blog posts detailing research I have done for that week.

# Marking Criteria

**PS**: Load a basic module into the kernel. Have it print statements on load and unload. Research and document (through blog, possibly on GitHub) methods by which rootkits/kernel processes can be detected. Document findings/research in a comprehensive manner.

**CR**: As above, plus: hook into the file system calls to prevent the user from seeing fake &ldquo;malicious files.&rdquo; (Not hiding the rootkit binaries, but actual user-level files.) Weekly blogs reflecting on learnings + progress.

**D**: As above, plus: hide the rootkit from the kernel (i.e. supress it from showing in data of running processes.)

**HD**: As above, plus: bypass a *Host-based Intrusion Detection Systems**. Have the rootkit undetectable to the elements such a system will search for.*

# *Timeline*

Week 1: Brainstorm ideas.

Week 2: Throw out first idea. Choose rootkit project with 3 days to go. Basic research and planning my approach to the rootkit.

Week 3: Setting up the FreeBSD operating system on a Virtual Machine. Create a basic kernel module.

Week 4: Create the function hook to modify the ls functionality (and its variants) such that target folders can be made invisible to the user (and as much to the operating system as plausible)

Week 5: Use Direct Kernel Object Manipulation to hide the presence of the rootkit from the user.

Week 6: Hide the presence of the rootkit from a HIDS.

Week 7: Unclear right now &ndash; maybe flexible time in case something comes up in a previous week. May also see if I can have the rootkit load silently from userland code. To be discussed.

Week 8: Final report due

Week 9: Presentation due