# Contributing to the IoTConnect C Library

We would love for you to contribute to our library and help make it even better than it is today!
As a contributor, here are the guidelines we would like you to follow:

 - [Submission Guidelines](#submit)
 - [Coding Rules](#rules)
 - [Commit Message Guidelines](#commit)
 - [Signing the CLA](#cla)


### <a name="submit-pr"></a> Submitting a Pull Request (PR)

Before you submit your Pull Request (PR) consider the following guidelines:

1. Be sure that an issue describes the problem you're fixing, or documents the design for the feature you'd like to add.
   Discussing the design upfront helps to ensure that we're ready to accept your work.

1. Make your changes in a new git branch.

1. Add your changes **including appropriate test cases**.

1. Follow our [Coding Standard Guidelines](#rules).

1. Run the full test suite, and ensure that all tests pass.

1. Push your branch to the repo.

1. In GitHub, send a pull request to `iotc-c-lib:master`.

### Reviewing a Pull Request

The IoTConnect team reserves the right not to accept pull requests at its sole discretion.

#### Addressing review feedback

If we ask for changes via code reviews then:

1. Make the required updates to the code.

1. Re-run the test suites to ensure tests are still passing.

That's it! Thank you for your contribution!

#### After your pull request is merged

After your pull request is merged, you can safely delete your branch and pull the changes from the main (upstream) repository.

## <a name="rules"></a> Code Formatting and Naming Conventions

To ensure consistency throughout the source code please follow these steps:

* All features or bug fixes must be tested by one or more unit-tests.
* Follow the existing naming conventions in the project.
* Format your code with CLion *CTRL-ALT-L* or Eclipse *CTRL-SHIFT-F* K&R Formatter with 120 character line wrap and 4 spaces indentation. 
* If you use another IDE, please ensure that your code is consistent with the existing coding style.
* Ensure that headers have ```#ifdef __cplusplus``` for C++ compatibility.
