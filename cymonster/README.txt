
See the full article at
http://labs.trolltech.com/blogs/2009/04/15/monster-evolution-in-qt-episode-3-revenge-of-the-cylinders/

This demo requires a working V8 library. V8 is the JavaScript engine for
Google Chrome. It is open-source and available from the official site:
http://code.google.com/p/v8/.

Please follow the instructions on Google V8 wiki page in order to build it.
It is expected that you have it under a sub-directory called v8. Some
important wiki pages are:
http://code.google.com/p/v8/wiki/Source
http://code.google.com/p/v8/wiki/BuildingOnWindows
http://code.google.com/p/v8/wiki/Contributing

Please pay attention to the required tools necessary to build V8 from source,
i.e. Python and scons.

Note: since last time I tested this demo is with revision 1686, it is then
suggested that you check out this particular revision.

Example: steps for Linux
------------------------

(1) First, we get the code for cymonster:

    git clone git://labs.trolltech.com/GraphicsDojo
    cd cymonster

(2) Next, we grab the V8 source-code:

    svn checkout -r 1686 http://v8.googlecode.com/svn/trunk v8

(3) Under OpenSUSE, it is necessary to disable ENV variable:

    unset ENV

(4) Now we trigger the build:

    cd v8
    scons mode=release processor=ia32 toolchain=gcc

(5) Finally, we can compile and run the example:

    cd ..
    qmake
    make
    ./cymonster -graphicssystem opengl

Note: the ENV problem is filed here:
http://code.google.com/p/v8/issues/detail?id=302


Example: steps for Windows with MinGW
-------------------------------------

Start by opening the Qt 4.5.0 command prompt.

(1) First, we get the code for cymonster:

    git clone git://labs.trolltech.com/GraphicsDojo
    cd cymonster

(2) Next, we grab the V8 source-code:

    svn checkout -r 1686 http://v8.googlecode.com/svn/trunk v8

(3) Edit v8/src/platform-win32.cc according to the patch in
    http://code.google.com/p/v8/issues/detail?id=309

(4) Now we trigger the build:

    cd v8
    scons mode=release processor=ia32

(5) Finally, we can compile and run the example:

    cd ..
    qmake
    mingw32-make
    release\cymonster -graphicssystem opengl

If scons fails to run (step 4), make sure your path is set correctly.
For example, you might need the following command first (depending
on your Python installation):

    set PATH=C:\Python26;C:\Python26\Scripts;%PATH%;

Using GUI tool e.g. TortoiseSVN to check out V8 is also possible.
Make sure you enter the right information (like in step 2) and that
it checks out to a sub-directory called v8.
