Notes from 6.1.2022 : Sertac

# This directory is extremely important

I seached for the latest source code of mvmid but I couldn't find. 
It seems KB did not give me before he leaves.

I requested sources from him again, but he did not reply yet.

There is a statically compiled library in this directory, called as libmvmid.a
In my original backup, src directory and makefile was non-existent
So the build system was just picking that file, and link with Mvapp

So I found a very old version of mvmid directory from archieves and found
src dir and makefile inside. Copied them to here, and tried building

Build failed because 3 functions were declared but not implemented:
- BcdStr2Word
- StrNcpy
- Str2Lower

So I wrote implementation code for these 3 functions, and recompiled without warnings
But please note that, BcdStr2word implementation doesn't seem to be correct to me
Need to fix this in future.

The size of the original libmvmid.a file is 17740.
When I implemented above 3 functions and compiled, my libmvmid.a size is 16932
So it seems I am still missing some other things here.

Therefore, I will keep original libmvmid.a here until I make sure my version prevails.

# History
Actually, KB created this file and put below content. 

* KB Kim
* Create for CVS Server
