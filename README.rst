=====
 HPM
=====

Rationale
---------

Sometimes you want to install a package by hand, you may want the git version
or you may want to compile it with different flags than your distro. What you
do in such cases is something like:

.. code-block:: bash

    ./configure --prefix=$HOME/.local
    make
    make install

But then later, you don't want the package anymore because your distro is more
up to date or because the dependencies are now broken. If you have deleted the
sources directory, you are screwed. If your package didn't implement (or badly
implemented) some ``make uninstall`` command, then again you're screwed.

What is it?
-----------

HPM (for Home Package Manager) is a small tool I wrote to have a (very simple)
package manager managing the packages you install by hand. It keeps track of
all the files a package installed (in a specific prefix) and allow you to
remove all of them in one command.

Installation
------------

You can install HPM through HPM itself with:

.. code-block:: bash

    make
    ./hpm install hpm "make install PREFIX=$HOME/.local"

And uninstall it later with:

.. code-block:: bash

    hpm uninstall hpm

Usage
-----

There are only two commands:

.. code-block:: bash

    hpm install <packagename> <command>
    hpm uninstall <packagename>

``packagename`` is yours to decide and ``command`` is the install command to
monitor for installed files.

Example
-------

Let's say you want to install the last git version of LLVM in your home, you
would do something like:

.. code-block:: bash

    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local -DCMAKE_BUILD_TYPE=Release
    make
    hpm install llvm 'make install'

And then later, if you want to delete it:

.. code-block:: bash

    hpm uninstall llvm

And that's all!

Licensing
---------

HPM is based on installwatch which is licensed under the GPL2 license provided
in LICENSE_GPL2. The hpm shell script is licensed under the FreeBSD license
provided in LICENSE_BSD.
