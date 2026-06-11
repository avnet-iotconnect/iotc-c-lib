# General Coding Standard Guidelines for all Avnet Projects

Scope: Files that we own and/or create.

Ensure that this document is read by you and/or your AI agent when working on non-C Avnet projects, 
Refer to [STYLE-C.md](STYLE-C.md) for C projects.

## General Philosophy

When writing or reformatting a file, evaluate what you may identify to be a reasonable conscious decision to violate a rule vs. a simple mistake, laziness or AI slop. Readability over consistency. Examples:
- A repeat `if failed then go to error handler` can go on one line if the same thing repeats often enough to warrant a one-liner.
- An above example may warrant an exception to a curly brace requirement in C.
- Third party code uses a different style, but you want to keep it as is for easier future merges with the third party code. Or perhaps you are making smaller changes on top of third party code.

In other words, any conscious exceptions are acceptable, **but slop is not**. But don't push your patterns in code "just because you like it" - hence this and other STYLE documents.

## Specific Guidelines

* For items not mentioned in this or related docs, consult examples in this repo, pinned "flagship" Avent repos, or use your best judgment.
* When editing README.md and other high churn file files, do not use Git's WYSIWYG editor. For there types of files lines to less than 120 characters. Break sentences into multi-line. The intent here is to easily see GIT diffs.
* For Python projects, use typing hints whereever possible and reasonable, especially for public APIs. This helps IDE and AI completions. Adopt STYLE-C.md guidelines for Python projects where applicable, if in doubt.
* For new Avnet projects, default the existing naming conventions in this project where possible and appropriate.
  * For all other files, directories and shell scripts (where applicable or suitable) except for Markdown files at top level use kebab-case.
  * For Markdown files at repository top level use UPPER_SNAKE_CASE.
  * All file extensions should be in lower case.
* Avoid unnecessary comment blocks for the sake of just having a comment block. For example, if function is called ```walk_animal()``` a comment block stating that "The function walks an animal" is redundant. So use self-describing function names and reserve comments for important things, so that the users actually read them.
* AI agents must refrain from extensively commenting thought process. Focus on commenting the code or algorithms.,
* Use descriptive function and variable names so that comments are not needed. Prefix boolean variables and function with "is_", "can_", "are_", or similar prefixes. Prefix getters/setters with "get_"/"set_". Match "create" with "destroy", "init" init "deinint". Avoid using generic names like "tmp", "temp" etc.
* Do not use any prefixes that designate type of the variable (struct x_my_struct, unsigned long ul_my_var).
* If your file is longer than 400 characters, consider re-designing your code to split the file into functionally related components.
* Attempt to conform libraries and SDKs with most pedantic but modern and reasonable settings. c99 standard option is a good for libraries, since this can help with integrations projects that have stricter compiler definitions.
* Always eliminate sensible warnings (like unused variables, initializations, etc.) in your code. If using a third party baseline/sample project, make an honest attempt to ensure that adding your code to the project does not introduce new warnings or introduce new warnings WRT the baseline's compiler/interpreter settings.
