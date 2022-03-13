# Tripwire: Fooling an HIDS


The final part of my plan is to try to fool an HIDS. Put briefly, a HIDS is a Hardware-based Intrusion Detection System. It will look at the files in your system (depending on your config) and scan them for additions, removals and modifications to your database of still valid files. In this case, I'm going to just be using tripwire, simply because the FreeBSD rootkit book mentioned it and it didn't really matter which one I used.

## Setting Tripwire Up

Tripwire is really made for deployment to proper, large-scale systems, and as such has a lot of security measures in place I don&rsquo;t particularly care about but that we still need to set up. This firstly involves setting up a site key and a global key, as tripwire encrypts all its policy and database files.

We do this with the `twadmin -m G -L local.key -S site.key` command. This will prompt us for a key password, and then they will be generated.

We then need a policy file &ndash; the thing which tells Tripwire what we want to check, and how we want to check it. My policy file looks like this:

I just told it to scan everything. Why not. I think that means if any file changes anywhere it&rsquo;ll ping me, but at this point I&rsquo;m done with this VM and I just need this to work, so I want to check everywhere.

Then we need to generate the database of the file system with its integrity still in place. We do that using `tripwire -m I`.

## Fooling the HIDS

When we check all dirs. for file addition, modification or removal, tripwire will by default pick up on the presence of our (fake in this case) virus binary in /sbin.

![](https://www.openlearning.com/u/callumjones/blog/SaHidingFromAHids/Picture18.png?action=download)

Now we load the rootkit. As we coded it to, it will hide its own presence from kernel, and also hide not_a_virus from any getdirentries calls. Having loaded the KLD, and then running tripwire again, we can see that it&rsquo;s now gone:

![](https://www.openlearning.com/u/callumjones/blog/SaHidingFromAHids/Picture19.png?action=download)

Awesome! It&rsquo;s now properly hiding the virus from the OS and acting as a proper malware support tool.

However, I think this is just checking for file changes and additions. Let&rsquo;s try making tripwire check for as much as possible. We change our policy file to include:

We then remove our trojan from /sbin and then regen the database with the new policy. Having done this, we get a clean check without the virus. Looks all fine, gives us this:

![](https://www.openlearning.com/u/callumjones/blog/SaHidingFromAHids/Picture20.png?action=download)

Now with the virus in /sbin but without the rootkit:

![](https://www.openlearning.com/u/callumjones/blog/SaHidingFromAHids/Picture21.png?action=download)

Now with the virus in /sbin but with the rootkit

![](https://www.openlearning.com/u/callumjones/blog/SaHidingFromAHids/Picture22.png?action=download)

So, we can see it does detect something has changed in sbin, but it doesn&rsquo;t realise something&rsquo;s been added. Now, as a final act, let&rsquo;s figure out what the change is that&rsquo;s pinging it, and how we could bypass it.

Originally, I thought that this ping hadn't happened in my first scan, but it turns out I just missed it before.

Basically, the issue here is that it isn&rsquo;t the existence of the file tipping off Tripwire &ndash; it&rsquo;s the fact that when we actually load the virus file into its folder (In this case, sbin), the folder has stats updated. More specifically:

* Folder access time.

* Folder modification time.

* Change time.

Sadly, I don&rsquo;t have the time to implement these changes, but I&rsquo;ll briefly go over the process of what I could do to rectify it:

* We fetch the stat of the target folder (here /sbin), and we store the results into a struct stat.

* We then have a struct timeval into which we load the time values inside stat. We then modify

* Then we call utimes to load the old time back into sbin after we&rsquo;ve dnoe whatever we wanted to SBin (i.e. load the file into there).

* We can&rsquo;t change the change time as easily (it&rsquo;s whole point is that it records any file change). To achieve this, we would need to patch the ufs_itimes function so that change times aren&rsquo;t update, after which we would then load our trojan file in with.

<span> </span>

## Reflections on this Part

This part was again, much more approachable than distinction. I enjoyed it, though, because it let me take a bit of a step back from the actual code of the rootkit and see how it would actually operate with software specifically meant to pick up things like that.

While I would have loved to make it completely invisible to a HIDS like this, I think it was good of me to acknowledge that I just didn't have the time. This was a good decision in the end, as it gave me the chance to actually put a really good effort into these write-ups.

I'm also happy with the fact that even though I didn't have time to finish, I still wrote up about what I would have done. This shows that I still put in research and thought, rather than just leaving this last section for dead and running.

And there it is! My Something Awesome - done in all its glory. I'm over the moon with how it turned out - I've learnt so much since I started, especially about how the kernel interacts with userspace and how it's regulated. For someone who's had no experience in OS before, I'm beyond proud of what I've achieved, and can't wait to show it off.