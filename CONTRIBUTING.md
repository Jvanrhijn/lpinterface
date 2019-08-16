# Contribution guidelines

## Style concerns

Follow the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).
Use clang-tidy to lint your code, and clang-format for formatting. Code formatting rules follow
the Google style guide, with the following exceptions: functions are snake_case, enum variants
are CamelCase, and exceptions are allowed.

Write documentation using doxygen. It will be automatically deployed when the code is merged
into the master branch.
