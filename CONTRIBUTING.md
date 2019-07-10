# Contribution guidelines

### Style concerns

Follow the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).
Use clang-tidy to lint your code, and clang-format for formatting. Code formatting rules follow
the Google style guide.

### Error handling

Don't use exceptions, since most solver backends also don't. Instead, return error codes. Use
enum classes to define error types, never plain enums.

