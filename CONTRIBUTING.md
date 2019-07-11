# Contribution guidelines

### Style concerns

Follow the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).
Use clang-tidy to lint your code, and clang-format for formatting. Code formatting rules follow
the Google style guide.

### Error handling

Don't use exceptions, since most solver backends also don't. To avoid making too much of a 
C-like API, we will use [tl::expected](https://github.com/TartanLlama/expected) to return error
values. tl::expected is essentially the same as std::experimental::expected, but with some
functional extensions similar to Rust's Result<T, E>.

