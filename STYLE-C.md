# Avnet C Coding Style

Scope: Files that we own and/or create.

Ensure that this document is read by you and/or your AI agent when working on C Avnet projects.

## General Philosophy

When writing or reformatting a file, evaluate what you may identify to be a reasonable conscious decision to violate a rule vs. a simple mistake, laziness or AI slop. Readability over consistency. Examples:
- A repeat `if failed then go to error handler` can go on one line if the same thing repeats often enough to warrant a one-liner.
- An above example may warrant an exception to a curly brace requirement in C.
- Third party code uses a different style, but you want to keep it as is for easier future merges with the third party code. Or perhaps you are making smaller changes on top of third party code.

In other words, any conscious exceptions are acceptable, **but slop is not**. But don't push your patterns in code "just because you like it" - hence this and other STYLE documents.


## Universal Guidelines

-For items not mentioned in this or related docs, consult examples in avnet-iotconnect/iotc-c-lib GitHub repo, pinned "flagship" Avent repos, or use your best judgment.
-When editing README.md and other high churn file files, do not use Git's WYSIWYG editor. For there types of files lines to less than 120 characters. Break sentences into multi-line. The intent here is to easily see GIT diffs.
-AI agents must refrain from extensively commenting their thought process. Focus on commenting the code or algorithms and using descriptive names
-If your file is longer than 400 characters, consider re-designing your code to split the file into functionally related components.
- Always eliminate sensible warnings (like unused variables, initializations, etc.) in your code. If using a third party baseline/sample project, make an honest attempt to ensure that adding your code to the project does not introduce new warnings or introduce new warnings WRT the baseline's compiler settings.

## Formatting

- Indent with 4 spaces. No tabs in new/edited lines.
- Use K&R/1TBS braces for functions and control blocks:

- Target readable width: up to about 160 columns. Over 160 is allowed for "I don't care to see this code often" for example ona long cookie-cutter variable assignment or declaration.
  - Keep short calls/conditions on one line.
  - Column width is a readability guideline, not a hard limit.
  - If a line gets long, use this continuation form (not stair-step) when more than a handful of args need to be broken up (case when many args):

```c
static void run_task(void *arg) {
    if (arg == NULL) {
        return;
    }
}
```

- `if`/`else` and similar generally should not omit curly braces and the braced statements should not be on the same line.
  - Exception is allowed for one-liner statements (typically error handling) that are repeated often enough to warrant it. e.g. `if (error) goto cleanup;` or `if (error) return error;
- Allowed Exception: Brace may be on a line by itself following the function definition, if a part of an official library that we provide or if preferred by the developer should be consistent.

- If needed, breaking up long calls/conditions should be done like examples below. Argument grouping is allowed, but not preferred for function declarations.
```c
result = some_function(
    arg1,
    arg2,
    arg3
);
```

```c
if (
    cond_a &&
    cond_b &&
    cond_c
) {
    do_work();
}
```

- Preferred alternative for printf format and when only a few args or grouping improves readability:

```c
printf("Result: %d, error: more long text ... %d\n",
   result, error_code
);

```
- AI should not collapse already-readable multiline calls into a single long line.

- Aim for condensed code (less LOC). Avoid empty lines, but add them to accent logical grouping or decoupling.
- It is acceptable to exceed 160 when breaking the line would reduce readability or add noisy wrapping.
- The general wrapping rule is "do I really need to see what's far to the right" or will I benefit more from seeing more important LOCs. 
- Typical exception: long `printf(...)`/logging calls where arguments are straightforward and already easy to scan especially when many other printfs in the file.
- Wrap lines when structure must be visually parsed (long conditions, nested calls, many non-trivial arguments, or mixed expressions).


# Naming

-  For C source and header file names, use `snake_case`.
  - functions/variables/files: `snake_case`
  - type names (`typedef struct`, enums, fn-pointer typedefs): `CamelCase` for public API structs.
  - macros/constants: `UPPER_SNAKE_CASE`
  - Header file function names and types should be prefixed with module name to avoid global collision.
- Exception Allowed: For example, for type names `snake_case` or `snake_case_t` is allowed if it really better aligns with vendor code style, 
but the exception should be avoided when code (or code pattern) is intended to be shared across different vendor platforms (an SDK or common libs).

- For general files (directories, scripts, documents, images, etc.) use kebab-case where not conflicting with C rules or rules below:
  - For Markdown files at repository top level use UPPER_SNAKE_CASE.
- All file extensions should be in lower case.

- Use descriptive function and variable names so that comments are not needed. Prefix boolean variables and function with "is_", "can_", "are_", or similar prefixes. Prefix getters/setters with "get_"/"set_". Match "create" with "destroy", "init" init "deinint". Avoid using generic names like "tmp", "temp" etc.
- Do not use any prefixes that designate type of the variable (struct x_my_struct, unsigned long ul_my_var).

# General Style
- Prefer early returns for guard/error paths; use one `goto cleanup` block when resource unwinding is needed.
- `goto` is acceptable for error handling common cleanup.
- Keep one statement per line.
- Before deciding to write `(void)` for ignored function return - think. Do you really need compiler compliance? 
Do you expect this to sometimes actually fail to warrant your intent to communicate ignored return for production perhaps?
- Do communicate ignored function args in function definitions with `(void)`.
- Declare in-function (stack) variables near where they are used, rather at the top of the function, unless there is a readability gain.
- Avoid declaring non-reusable constants just for the sake of having a constant, especially at file scope. vTaskDelay(DELAY_MS) is less readable than having an actual literal. Supplement with a comment where needed or use a local const.
- Avoid using fixed size types (like uint32_t, int16_t etc.), where possible. Use them only intentionally if you feel that underlying layer use/requirement can be broken, if you are intending to pack values into storage or compose binary network packets. Use conversions/casts where it is acceptable.
- Grouping of related args/statment on the same line is allowed when logically related (eg. the printf example below).


## Headers

- New C/H files we own use SPDX header:

```c
/* SPDX-License-Identifier: MIT
 * Copyright (C) 2026 Avnet
 * Authors: Author Name <author.name@avnet.com> et al.
 */
```

- Agents should pull author info from git global config.
- When editing vendor files, keep vendor header and append SPDX line if needed.
- Ensure that headers have ```#ifdef __cplusplus``` for C++ compatibility for portable projects that are not restricted to C only.

## Comments

- Do not do docstring/javadoc style header comments.
- Describe the non-obvious contract condition in function declaration header. Especially conditions involving memory ownership or responsibility to free allocated results or globals.
- Use // for most comments. Use /* */ when the comment has internal structure — bulleted/dashed lists, multi-paragraph prose, or a flow walkthrough. Plain prose stays // even if it spans several lines. Avoid lone * continuation lines.
- AI should not use numbered steps when describing flow.
- Comment the why, not the obvious what.
- Avoid unnecessary comment blocks for the sake of just having a comment block. For example, if function is called ```walk_animal()``` a comment block stating that "The function walks an animal" is redundant. So use self-describing function names and reserve comments for important things, so that the users actually read them.
- Comment complex algorithms in implementation. Use comment header blocks to explain the flow and intent.
- When API call (usually in headers) is complicated provide call examples.
- Keep comments short and local to non-obvious logic.
- Never use block comments with standard "parameters" and "returns" unless a library code and probably not even then.

## Other
- No vertical alignment padding (types, names, assignments, args). Use single spaces only.
- Exception to the above: A list of #define constants can be aligned if repeats enough.

## Legacy/reflow policy

- Avoid drive-by reformatting.
- Reflow only touched blocks.
- Never perform style-only mass rewrites in functional patches.

