# Contributing to the IoTConnect C Library

We would love for you to contribute to our library and help make it even better than it is today!
As a contributor, we would like you to follow the guidelines laid out in this document. 

### Submitting a Pull Request (PR)

Before you submit your Pull Request (PR) consider the following guidelines:

1. Be sure that an issue describes the problem you're fixing, or documents the design for the feature you'd like to add.
   Discussing the design upfront helps to ensure that we're ready to accept your work.
1. Before making any changes, ensure that you read [STYLE-C.md](STYLE-C.md) and follow the coding standards described there.
1. If using an AI agent to make changes, tell your agent to execute [AGENTS_BOOTSTRAP.md](AGENTS_BOOTSTRAP.md) before making any changes, and ensure that [AGENTS.md](AGENTS.md) is being loaded in every session.
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

