.. _gettingstarted:

Getting Started
===============

Environment Set-up
-------------------

Make sure the ``~/.local/bin`` is in your ``$PATH`` by running 

.. code-block:: bash 

  $ echo $PATH | grep $HOME/.local/bin | wc -l

If the output is not 0, you are good to go. Else add 

.. code-block:: bash 

  export PATH=$PATH:$HOME/.local/bin

to your shell's configuration files (``~/.bashrc`` or ``~/.zshrc``). Remember to either reopen you shell or source the rc file.  

.. code-block:: bash 

  $ source ~/.bashrc # if you use bash
  $ source ~/.zshrc # if you use zsh 

Library Set-up
---------------------

Let's start by cloning the git repository

.. code-block:: bash 

  $ cd </path/to/your/library/folder>
  $ git clone https://github.com/cunbidun/cpcli.git

Install and move the newly compiled binary to ``~/.local/bin``.

.. code-block:: bash 

  $ cd cpcli
  $ make all
  $ make install

If the installation process finishes successfully, you can see the version of ``cpcli_app`` by running

.. code-block:: bash 

  $ cpcli_app -v


Workspace Set-up
---------------------

Workspace is where you solve the problems.

.. code-block:: bash 

  $ cd </path/to/your/workspace/folder>
  $ mkdir task 
  $ mkdir output 
  $ mkdir template
  $ mkdir archive
  $ mkdir include
  $ touch project_config.json

Make sure you create this file structure:

.. code-block:: text 

  /path/to/your/workspace/folder
  ├── task
  ├── output
  ├── template
  ├── include
  ├── archive
  └── project_config.json

Put this inside the 	``project_config.json``

.. code-block:: json 

  {
    "frontend_exec": "java -jar </path/to/your/library/folder>/cpcli/binary/frontend/Test.jar",
    "task_dir": "</path/to/your/workspace/folder>/task",
    "output_dir": "</path/to/your/workspace/folder>/output",
    "template_dir": "</path/to/your/workspace/folder>/template",
    "archive_dir": "</path/to/your/workspace/folder>/archive",
    "include_dir": "</path/to/your/workspace/folder>/include",
    "cpp_compiler": "g++",
    "cpp_compile_flag": "-DLOCAL -O2 -std=c++17",
    "cpp_debug_flag": "-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG",
    "use_precompiled_header": false,
    "use_cache": true
  }