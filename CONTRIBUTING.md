# Contributing to the IoTConnect C Library

We would love for you to contribute to our library and help make it even better than it is today!
As a contributor, we would like you to follow the guidelines laid out in this document. 

### Submitting a Pull Request (PR)

Before you submit your Pull Request (PR) consider the following guidelines:

1. Be sure that an issue describes the problem you're fixing, or documents the design for the feature you'd like to add.
   Discussing the design upfront helps to ensure that we're ready to accept your work.

1. Make your changes in a new git branch.

1. Add your changes **including appropriate test cases**.

1. Follow our Follow our Coding Standard Guidelines.

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

To ensure consistency throughout the source code please follow these steps:

* All features or bug fixes must be tested by one or more unit-tests.
* Follow the existing naming conventions in the project.
* The project uses GTK/Gnome naming convention and K&R 1TBS (OTBS) Formatting, with acceptable OTBS exceptions like iotcl_telemetry_create where full conformance would reduce readability.
* Format your code with CLion *CTRL-ALT-L* or Eclipse *CTRL-SHIFT-F* K&R Formatter with 120 character line wrap and 4 spaces indentation. 
* If you use another IDE, please ensure that your code is consistent with the existing coding style.
* Ensure that headers have ```#ifdef __cplusplus``` for C++ compatibility.
* Avoid using fixed size types (like uint32_t, int16_t etc.), where possible. Use them only if if you feel that underlying layer use/requirement can be broken, if you are intending to pack values into storage or compose binary network packets. Use conversions/casts where it is acceptable.
* Avoid unneccessary comment blocks for the sake of just having a comment block. For example, if function is called ```walk_animal()``` a comment block stating that "The function walks an animal" is redundant. So use self-describing function names and reserve comments for important things, so that the users actually read them.
