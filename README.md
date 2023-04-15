# QOOM

Yes, it really is called "QOOM".

This is a fork for experimenting, tinkering, hacking and whatnot of the DOOM
source code, as was available in the [id-Software/DOOM][doomrepo] repository.

The code in the repository is for Linux DOOM, whereas I intend to get this to
compile and run under Windows *(it does not yet compile)*.

I intend on making small adjustments to the code. This may include some bugfixes
as well as backend changes *(e.g. moving from `fixed_t` to `float`s, see the
[linuxdoom TODO][linuxdoomtodo])*.

## Source structure

The source code was entirely restructured to fit my personal build system
([bip][bip]) better. This includes:

  * Splitting headers from their corresponding sources: [include](include),
    [source](source)
  * Moving some files that fall under the same pseudo-namespace into
    subdirectories: e.g. `r_sky.[ch]` -> `qoom/r/sky.[ch]`
  * Moving additional files present alongside the code into the proper
    [`docs`](docs) subdirectory: e.g. `linuxdoom-1.10/README` ->
    `docs/linuxdoom/README`
  * Reformatting files as I work on them, see [`.editorconfig`](.editorconfig).

[doomrepo]: https://github.com/id-Software/DOOM
[linuxdoomtodo]: docs/linuxdoom/TODO
[bip]: https://github.com/qeaml/bip
