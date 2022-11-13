.. _overview:

Overview
========================================================================================
This page provides some high-level overview about Competitive Programming CLI

What is this?
----------------------------------------------------------------------------------------
This project provides a set of tools for testing the correctness of solutions in coding competitions.
Because they are all command-line interface (CLI), the project is independent of text editors/ IDEs.
`Visual Studio Code <https://code.visualstudio.com/>`_ and `Neovim <https://neovim.io/>`_ 
are the recommended text editors. Check the :ref:`texteditor` for more information

This project is in the early stage of development, so there is a lot of room for improvement. 
Nevertheless, it is mature enough for casual coding competition.


How does ``cpcli`` test your solutions?
------------------------------------------------------------------
1. Compile every necessary file (only compile modified files).
2. Test the program in sample test cases and user generated test cases.
3. Generate more tests via a test case generator if applicable. 
4. Test the solution on newly generated test case.
