# Contributing to RaeenOS

First off, thank you for considering contributing to RaeenOS! This project is ambitious, and every contribution, from a small bug fix to a new device driver, is highly valued.

This document provides a set of guidelines to ensure that our collaboration is smooth and the codebase remains high-quality, stable, and maintainable.

## Submitting Changes

Please follow this process for all contributions:

1.  **Fork the repository** and create a new branch from `main` for your work.
2.  **Make your changes.** Ensure your code adheres to the guidelines below.
3.  **Test your changes thoroughly.** Build the ISO and run it in QEMU to ensure the system remains bootable and stable.
4.  **Update the documentation** if you are adding new features, changing APIs, or altering behavior.
5.  **Submit a pull request** with a clear title and a detailed description of your changes. Explain the "what" and "why" of your contribution.

## Development Philosophy

We adhere to a few core principles to guide our design decisions.

*   **KISS (Keep It Simple, Stupid)**: Solutions must be as straightforward as possible. In kernel development, complexity is a direct threat to stability. Avoid over-engineering.
*   **YAGNI (You Aren’t Gonna Need It)**: Do not add speculative features. Focus on the immediate, required functionality. The kernel should remain as lean as possible.
*   **SOLID Principles**:
    *   **Single Responsibility**: A module, class, or function should have one, and only one, reason to change. (e.g., a memory manager manages memory, a scheduler schedules tasks).
    *   **Open/Closed**: Kernel modules and subsystems should be open for extension (e.g., adding a new filesystem or driver) but closed for modification. Rely on well-defined interfaces.
    *   **Liskov Substitution**: A driver for a specific piece of hardware must be substitutable for its generic base driver interface without altering the correctness of the system.
    *   **Interface Segregation**: Prefer many specific interfaces over one general-purpose one. (e.g., separate interfaces for block devices, character devices, and network devices).
    *   **Dependency Inversion**: High-level modules (like the VFS) should not depend on low-level modules (like a specific disk driver). Both should depend on abstractions (like a `BlockDevice` interface).

## Code Quality & Style Guidelines

1.  **Error Handling is Critical**:
    *   Never assume a pointer is valid. Check all pointers from user space or hardware.
    *   Use assertions (`ASSERT(condition)`) to validate preconditions and invariants during development. These should compile out in release builds.
    *   For recoverable errors, return specific error codes. Do not fail silently.
    *   For unrecoverable errors, trigger a kernel panic with a descriptive message. The system must halt safely rather than continue in an unstable state.

2.  **Code Comments**:
    *   Use Doxygen-style comments for all public functions, classes, and structs.
    *   Explain the *why*, not the *what*. The code itself should explain what it's doing. Comments should explain why it's necessary.
    *   Document any non-obvious behavior, potential side effects, or hardware-specific workarounds.

3.  **C++ Features**:
    *   **No Exceptions/RTTI**: The kernel is compiled with `-fno-exceptions` and `-fno-rtti`. Do not use these features.
    *   **No Standard Library**: We cannot use the C++ standard library. All necessary data structures and utilities must be implemented within the kernel.
    *   **RAII**: Use Resource Acquisition Is Initialization (RAII) for managing kernel resources like locks, memory allocations, and disabling/enabling interrupts.

4.  **Memory and Concurrency**:
    *   Be mindful of stack usage. The kernel stack is small and fixed.
    *   All memory allocations must be checked for failure.
    *   Identify and protect shared data with appropriate synchronization primitives (spinlocks, mutexes, semaphores). Document why a particular lock is needed.

## Security Compliance

The kernel is the root of trust. Security is not optional.

1.  **Validate All Inputs**: Rigorously validate and sanitize all data received from user space (e.g., via system calls) or from hardware devices.
2.  **Principle of Least Privilege**: Code should only have the permissions it absolutely needs. Drivers should not have access to unrelated kernel subsystems.
3.  **No Hardcoded Secrets**: Do not store sensitive values directly in the source code.
4.  **Prevent Buffer Overflows**: Use safe string and buffer manipulation functions. Always check buffer sizes.
5.  **Kernel/User Space Separation**: Never blindly trust a pointer or a value from user space. Data must be explicitly and carefully copied between the two domains.

## System Extension and API Design

When adding new features, follow these rules to maintain a clean architecture.

1.  **Adhere to Existing Frameworks**: When adding a device driver, filesystem, or system call, follow the patterns and interfaces already established in the codebase.
2.  **Backward Compatibility**: Once an API (like a set of system calls) is stabilized, it should not be changed in a way that breaks existing user-space applications. Extensions are preferred over modifications.
3.  **Modularity**: Design new subsystems to be as self-contained as possible. This aids in testing, debugging, and maintenance.

## Testing and Quality Assurance

1.  **Test Your Code**: All changes must be tested. At a minimum, this means building the ISO and booting it in QEMU to ensure no regressions have been introduced.
2.  **Integration Testing**: For new features, describe the steps you took to test them in your pull request. This helps reviewers understand the scope of your testing.
3.  **Regression Tests**: As we build a testing framework, new features should be accompanied by regression tests to prevent future breakage.
4.  **Code Review**: All pull requests must be reviewed and approved by at least one other contributor before being merged. This is a mandatory step for maintaining code quality and sharing knowledge.

## Documentation

If your code introduces a new feature, you must also document it.

1.  **Code is Documented**: Public APIs must have Doxygen-style comments.
2.  **Features are Documented**: Major new features should have a corresponding entry in the `/docs` directory.
3.  **Code Snippets**: All code examples in documentation must be correct, tested, and reflect real use cases.

By following these guidelines, we can work together to build a truly revolutionary operating system. Thank you for being a part of this journey!