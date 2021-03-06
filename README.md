# Project Overview

This was an entirely self-directed project, where we could choose any security-based topic we liked
and work on that for the whole term as a major project. We had to do our best to document our work
consistently and methodically throughtout the term, and show what we learnt even if unable 
to finish everything we hoped to. The idea was to push yourself and learn something new.

I decided to write a kernel-level rootkit in C for the FreeBSD operating system. 
I wanted to do this because of its inherent difficulty and challenge, and because
I had never worked on something even remotely similar to this before. See [here](wk3_initial_proposal.md) for my initial proposal.

Below is the write-up I did at the end of the project, summarising the entire process, 
notes I took, difficulties I had and reflections I made.

# Final Summary

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">I designed a Rootkit for the FreeBSD system, whose purpose was to sit invisibly when loaded into the kernel and hide a fake &ldquo;virus&rdquo; file. This was a super technically challenging project, but I enjoyed it immensely and I&rsquo;m really proud to show off what I did.</span>

## Research Notes

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">Firstly, I wrote up comprehensive notes for everything I did, plus a little extra (syscall) so I was comfortable working with the system. This made sure that a) if I had any troubles, I could have an easy set of notes to refer to, b) proved that I did research, and c) gave me something to show for my effort even if I couldn&rsquo;t complete a section. See: [KLD Notes](wk4_module_loading_notes.md) | [Syscall Notes](wk5_system_call_notes.md) | [Function Hooking Notes](wk6_function_hooking_notes.md) | [DKOM Kernel Hiding Notes](wk8_dkom_notes.md) | [HIDS Notes](wk9_HIDS_hiding_notes_and_imlementation_writeup.md).

*   Sometimes I did research beyond what my project called for. However, this extra research actually made future sections much easier to understand - see note near end of this post, for example: [FH Reflections 1](wk6_function_hooking_implementation_writeup1.md)

## Progress Documentation, Difficulties & Solutions

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">What I&rsquo;m most proud of are my write-ups that I wrote as I worked through each deliverable. Originally, I would just do a weekly reflection on my progress. However, as the coding got more technically complex, these ended up becoming highly detailed documentation of what I did to turn the knowledge I had from my notes into an actual working piece of software. This included bugs and issues I ran into, how I solved them, etc. **If you have to choose any to read with more care, please choose the one marked with a (*) below**. (Be warned, it&rsquo;s like 13 pages - I had a lot that I tried and failed on in that section. :</span>

*   [Week 3 Reflection](wk3_reflection.md) | [Week 4 Reflection](wk4_reflection.md) | [Week 5 General Reflection](wk5_general_reflection.md) | [Week 5 Syscall Reflections](wk5_syscall_implementation_writeup.md) | [Function Hooking Reflections 1](wk6_function_hooking_implementation_writeup1.md) | [Function Hooking Reflections 2](wk7%268_function_hooking_implementation_writeup2.md)(*) | [DKOM Hiding Reflections](wk8_dkom_implementation_writeup.md) | [HIDS Hiding Reflections](wk9_HIDS_hiding_notes_and_imlementation_writeup.md)

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">The HIDS Hiding section was the only deliverable I couldn&rsquo;t fully complete, but I was nonetheless very proud of it. I acknowledged that I did not have the time to finish it and still made an effort to write up what I would have done (See HIDS Hiding reflections above).</span>


## Particularly Important Reflections

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">As I discuss in </span>[Function Hooking Reflection 2](wk7%268_function_hooking_implementation_writeup2.md), I spent a huge amount of time trying to hook the system call responsible for displaying the file system before realising there was a section discussing File Hiding in the Designing Rootkits for FreeBSD book. What this means is that there is a lot of work I did here which did not end up being applied to the final product, but which I think reflects a couple of things:</span>

*   <span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">An engineering mindset in my willingness to try things knowing that they may not work - see all the effort I put into things like stat() and mkdir, even though it didn&rsquo;t come to anything. This also showed an ability to accept when a solution likely wouldn&rsquo;t work and move on to something else, regardless of how much time I&rsquo;d already spent on that first solution.</span>

*   <span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">Good analytical process - working through an issue methodically, narrowing down problems and trying different approaches until I found one which works. In particular, my debugging approach in the "Weird Bug" section. This one was particularly interesting, because the bug existed in the code the FreeBSD book gave me, so there was no other help I could use.</span>

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">I&rsquo;m also particularly proud of the general improvement in knowledge I can show. I think this is already clear in part through the detail I put in my write ups, but for a particular example:

*   <span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">In the first write-up, I had the following question: how is moduledata name different to DECLARE_MODULE name when you load a user-written module into kernel (see the Week 3 Reflection here).
*    Thanks to my research over the term, I now know. Basically, the name variable in struct moduledata is a string that gets loaded into the linker_file data, as I learnt from my </span>[DKOM Hiding Reflections](wk8_dkom_implementation_writeup.md) (see my strcmp in the linker_file FOREACH loop code). The other name argument - the one in DECLARE_MODULE - isn&rsquo;t actually a variable at all - it&rsquo;s a C Precompiler thing, where anything with ##name in their code will get replaced with the name I type (see the DECLARE SYSCALL macro </span>[here](wk5_system_call_notes.md).

# Deliverables Completion

For all my deliverables, I said I would write comprehensive notes, as well as weekly write ups. As linked above, I think I have shown more than sufficient evidence for this. Here&rsquo;s the marking criteria I set (see [here](wk3_initial_proposal.md)).

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">As for each point:</span>

Pass: Definitely completed ([here](wk4_module_loading_notes.md)).

Credit: Definitely completed ([here](wk6_function_hooking_implementation_writeup1.md) | [here](wk7%268_function_hooking_implementation_writeup2.md)).

Distinction: Definitely completed ([here](wk8_dkom_implementation_writeup.md)).

HD: Got half-completion. I managed to hide it from most of the HIDS, but it still picked up that I had made some modification to the /sbin folder (with my dummy virus file), it just didn&rsquo;t know what I&rsquo;d modified. I reflected on this and still summarised what I would have done if I had time to solve this problem. ([Here](wk9_HIDS_hiding_notes_and_imlementation_writeup.md)).

# Professional Reflection

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">I think the first thing that is really evident is that throughout the term, I got much better at documenting my own progress, both the successes and failures. This also includes being detailed about the process and my reasons for doing things. I think this is a good skill to have developed for when I have to work in corporate teams, etc. At the very least, communication skills are improved.</span>

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">I also discussed this a bit in the 2nd function hooking write-up, but I think I&rsquo;ve improved a lot in terms of just being willing to try something, with no guarantee that something will come from it. I think this is a really important thing for my professional life, and it&rsquo;s definitely something I was a lot worse at before this assignment. I&rsquo;m sure I&rsquo;ll have plenty of times in the future where there is little help available for a particular problem, and I need to figure something out for myself through a process of trial-and-error. Knowing that failure is an inherent part of this process is a real help to me.</span>

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">Finally, I just want to reflect on how happy I am with this project. I know I&rsquo;ve said it a lot, but it was a super challenging task and it really pushed me. But you know what? I&rsquo;d do it all again. I think part of the reason it was so good was because it was so difficult. I&rsquo;ve never worked in OS before, and so I learnt a heap with it (as I think the sheet amount of reflection write ups I have show).</span>

<span style="font-size:11pt;background-color:transparent;font-style:normal;text-decoration:none;">Thanks for coming on the journey with me.</span>

 