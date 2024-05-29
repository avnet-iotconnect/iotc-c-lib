# Contributing to the IoTConnect C Library

We would love for you to contribute to our library and help make it even better than it is today!
As a contributor, we would like you to follow the guidelines laid out in this document. 

### Submitting a Pull Request (PR)

Before you submit your Pull Request (PR) consider the following guidelines:

1. Be sure that an issue describes the problem you're fixing, or documents the design for the feature you'd like to add.
   Discussing the design upfront helps to ensure that we're ready to accept your work.

1. Make your changes in a new git branch.

1. Add your changes **including appropriate test cases**.

1. Follow our Coding Standard Guidelines.

1. Run the full test suite, and ensure that all tests pass.

1. Push your branch to the repo. (Rebase your branch to the latest, PR will be rejected since it cannot be merged after review)

1. In GitHub, send a pull request to `iotc-c-lib:master`.

#### Reviewing a Pull Request

The IoTConnect team reserves the right not to accept pull requests at its sole discretion.

#### Addressing review feedback

If we ask for changes via code reviews then:

1. Make the required updates to the code.

1. Re-run the test suites to ensure tests are still passing.

That's it! Thank you for your contribution!

#### After your pull request is merged

After your pull request is merged, you can safely delete your branch and pull the changes from the main (upstream) repository.

### Coding Standard Guidelines

To ensure consistency throughout this repository, as well as SDKs and sample applications please follow these steps:

* For new projects, follow the existing naming conventions in this project where possible and appropriate.
* The project uses GTK/Gnome naming convention and K&R 1TBS (OTBS) Formatting, with acceptable OTBS exceptions like where full conformance would reduce readability.
* Follow the naming patterns, style, bracket and curly brace placement per how coding style is presented in this repo. The following list represents (but is not limited to) the most common style definitions:
   * For function definitions, curly brace can start in the same line as the function name or in a new line by itself (as long as consistent across the project).
   * For type names for structs, enums and function pointers, use CamelCase.
   * For all non-constant (static, global or local) variable names, function names and function parameters, use snake_case.
   * For C source and header file names, use snake_case.
   * For all other files, directories and shell scripts (where applicable or suitable) except for markdown files at top level use kebab-case.
   * For markdown files at repository top level use UPPER_SNAKE_CASE.
   * All file extensions should be in lower case.
   * Global constants and enumerations should be in UPPER_SNAKE_CASE, prefixed with IOTCL_ (in this repo), IOTC_ (in SDKs) and APP_ or IOTC_ (in applications). Constants in function scope may follow normal variable guidelines or constant guidelines.
   * Header file function names and types should also be prefixed as above (having appropriate case/style). 
* For items not mentioned above, consult examples in this repo, or use best judgement.
* For best results, Format your code with CLion *CTRL-ALT-L* or Eclipse *CTRL-SHIFT-F* K&R Formatter with at least 120 (or unlimited) character line wrap and 4 spaces indentation. As many developers use different IDEs mixing spaces and tabs in code is acceptable, as long as tab width is aligned with 4 spaces, when spaces are present in the files.
* Ensure that headers have ```#ifdef __cplusplus``` for C++ compatibility for portable projects that are not restricted to C only.
* Avoid using fixed size types (like uint32_t, int16_t etc.), where possible. Use them only if if you feel that underlying layer use/requirement can be broken, if you are intending to pack values into storage or compose binary network packets. Use conversions/casts where it is acceptable.
* Avoid unnecessary comment blocks for the sake of just having a comment block. For example, if function is called ```walk_animal()``` a comment block stating that "The function walks an animal" is redundant. So use self-describing function names and reserve comments for important things, so that the users actually read them.
* Use descriptive function and variable names so that comments are not needed. Prefix boolean variables and function with "is_", "can_", "are_", or similar prefixes. Prefix getters/setters with "get_"/"set_". Match "create" with "destroy", "init" init "deinint". Avoid using generic names like "tmp", "temp" etc.
* Do not use any prefixes that designate scope or type of the variable (avoid g_global_var, struct x_my_struct, unsigned long ul_my_var).
* If your file is longer than 400 characters, consider re-designing your code to split it into functionally related components.
* Attempt to conform libraries and SDKs with most pedantic settings and c99 standard option, since this can help with integrations projects that have stricter compiler definitions.
* Always eliminate sensible warnings (like unused variables, initializations, etc.) in your code. If fusing a baseline/sample project, make an attempt witin reason to ensure that adding your code to the project does not introduce new ones or warnings or introduce warnings of a new type.
