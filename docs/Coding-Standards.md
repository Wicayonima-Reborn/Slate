# Coding Standards

## C++23
- Use modern C++ features (concepts, ranges, constexpr, etc.)
- Avoid legacy C patterns (raw pointers, manual memory management)
- Prefer std::unique_ptr and std::vector over manual allocation

## Architecture Rules
- **No global mutable state**
- **No singletons** (use explicit context where needed)
- **Prefer composition over inheritance**
- **All public APIs documented** (header comments)
- **Use pimpl idiom** for ABI stability where needed

## Naming Conventions
- Classes: PascalCase (ApplicationContext)
- Functions: snake_case (init_logging)
- Namespaces: snake_case (slate::core)
- Constants: kConstantName or UPPER_CASE
- Member variables: suffix _ (pimpl_, db_)

## Formatting
- clang-format with project .clang-format (to be added)
- Braces on newline (Allman style)
- 4 spaces indent

## Linting
- clang-tidy with checks: ugprone-*, modernize-*, performance-*

## Testing
- doctest framework
- Every module must have corresponding unit tests (target 	ests/)
- Integration tests for CLI commands
