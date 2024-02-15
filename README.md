## Intro

This was a simple Frogger clone I wrote years ago for fun.  I never fully finished it as it's missing a few items from the original game but it is fully playable and you can beat the level. 

## Surprisingly, this will build on a modern Ubuntu 22.04 system by running the following:

    $ sudo apt-get install -y libsdl1.2-dev libsdl-image1.2-dev libsdl-ttf1.2-dev libsdl-ttf2.0-dev libsdl-mixer1.2-dev
    $ ./autgen.sh
    $ ./configure
    $ make
    $ src/froggix
