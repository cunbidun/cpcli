Language:

default_language flag: 
This flag set the default_language for the project. When new task is created, the solution file 
with be using this language. In the future, we can have this config:

.. code-block:: json 

  {
    "default_language": {
      "solution":"cpp,
      "slow":"python", 
      "gen":"rust",
      "checker":"java",
      "interactor": "python"
    }
  }

if a file type is missing from default_language, it's will be set to cpp

Specific language config:
We probably need a complier manager class. The main method of the class will be "compile". Its main functionality is 
to compile the source code given a type (i.e. solution, slow, gen, checker). THe language will be inferred from the 
file extension. 
-> allow for support cpp solution with python generator