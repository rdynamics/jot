# jot
jot is a simple program to pack images into a single spritesheet for use with Retro Dynamics.
Right now it does not use a particularly sophisticated algorithm, but it does work.

## compiling
jot has no dependencies other than `stb_image`, which is included in the directory. Compilation is therefore simple: `cc jot.c -o jot`.

## usage
As current functionality is very limited, jot is very simple to use right now. Specify a list of files, and jot will generate two C files:
"images.h" and "images.c". "images.c" contains C code to load the spritesheet into an OpenGL texture, and "images.h" contains structures
that specify the coordinates for each image in the spritesheet.

Two image files are included under `demo`; jot can be demoed by `cd`'ing into `demo` and running `jot demo1.png demo2.png`.

## todo
- [ ] Generate non-square images to prevent wasted space
- [ ] Actually test with OpenGL to make sure images are generated correctly
- [ ] Auto-detect components so that e.g. RGB images are saved correctly
- [ ] Extend kinds of generated C structures: eventually include animation generation etc
- [ ] Make it easy to generate files based on a standard file structure (probably images under resrc/img)
