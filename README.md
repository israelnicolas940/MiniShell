# Mini-Shell - Operating Systems Course

A simplified command-line interpreter implementation demonstrating fundamental process management concepts in Unix/Linux systems.

## Project Overview

This project implements a basic shell that consolidates core operating systems concepts including process creation, management, and synchronization. The mini-shell follows the standard shell execution cycle and supports both internal commands and external program execution.

### Learning Objectives

- Understand `fork()`, `exec()`, and `wait()` system calls
- Implement process creation and synchronization
- Manage foreground and background processes
- Handle zombie process prevention
- Apply command parsing and interpretation concepts

## Features

### Required Functionalities
- **Interactive Prompt**: Displays `minishell> ` for user input
- **External Commands**: Execute system commands (`ls`, `date`, `whoami`, etc.)
- **Internal Commands**:
  - `exit` - Terminate the shell
  - `pid` - Display shell PID and last child process PID
- **Error Handling**: Proper system call return value verification
- **Command Parsing**: Support for multiple arguments
- **Well-structured Code**: Clean, commented implementation

### Optional Functionalities
- **Background Execution**: Run commands with `&` symbol
- **Job Management**: 
  - `jobs` command to list background processes
  - `wait` command to synchronize all background processes
- **Process Tracking**: Monitor running background jobs with status updates

## 🛠️ Technical Implementation

### Core System Calls Used
- `fork()` - Process creation
- `execvp()` - Program execution
- `wait()/waitpid()` - Process synchronization
- `getpid()` - Process identification

### Key Concepts Demonstrated
1. **Process Creation**: Using `fork()` to create child processes
2. **Program Execution**: Replacing process image with `exec()` family
3. **Process Synchronization**: Parent-child coordination with `wait()`
4. **Background Processing**: Non-blocking command execution
5. **Zombie Process Prevention**: Proper cleanup of terminated processes

## Project Structure

```
mini-shell/
├── src/
│   └── mini_shell.c          # Main implementation
├── examples/                 # Reference examples
│   ├── fork.c
│   ├── fork-print.c
│   └── fork-execve.c
├── docs/
│   └── technical_report.pdf  # Implementation analysis
└── README.md
```

## Compilation and Usage

### Prerequisites
- GCC compiler
- Unix/Linux environment
- Standard C libraries
- CMake
### Configuration
```bash
# from the project root
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
- Available build types: Debug, Release.
- You only need to specify the build type in the first configuration.
- If you want to change it later, re-run cmake with a new -DCMAKE_BUILD_TYPE value.

### Compilation
```bash
cmake --build build/
```
### Running 
```bash
./build/mini_shell
```
### Debugging
```bash
gdb ./build/mini_shell
```
### Testing 
If GoogleTest is available, unit tests will be build automatically. Run all tests from the project root with:
```bash
ctest --test-dir build
```
Or run the executable directly:
```bash
./build/unit_tests
```

### Example Usage
```bash
minishell> ls -la
minishell> date
minishell> sleep 10 &          # Background execution
[1] 1234
minishell> jobs                # List background jobs
[1] 1234 Running
minishell> pid                 # Show process IDs
Shell PID: 1000, Last Child PID: 1234
minishell> wait                # Wait for background processes
[1]+ Done
minishell> exit
```

## Testing

### Basic Tests
- Execute simple commands (`ls`, `date`, `whoami`)
- Test internal commands (`pid`, `exit`)
- Verify error handling with invalid commands

### Advanced Tests
- Background process execution
- Multiple background jobs
- Job listing and status tracking
- Process synchronization with `wait`

### Test Commands
```bash
# Basic functionality
minishell> ls
minishell> date
minishell> pid
minishell> exit

# Background processing
minishell> sleep 10 &
minishell> jobs
minishell> sleep 5 &
minishell> jobs
minishell> wait
minishell> jobs
```

## Architecture

### Process Flow
1. **Initialize**: Display startup message and shell PID
2. **Main Loop**:
   - Display prompt
   - Read user input
   - Parse command and arguments
   - Determine command type (internal/external)
   - Execute appropriate handler
   - Clean up finished background processes

### Background Process Management
- Maintains array of background process PIDs
- Periodic cleanup of terminated processes
- Non-blocking status checks with `WNOHANG`
- Job numbering and status reporting

## Implementation Details

### Key Functions
- `parse_command()` - Command line parsing and argument extraction
- `execute_command()` - Process creation and execution handling
- `is_internal_command()` - Internal command detection
- `handle_internal_command()` - Internal command execution
- `clean_finished_processes()` - Zombie process cleanup

### Memory Management
- Static arrays for argument storage
- Dynamic background process tracking
- Proper string handling and bounds checking

## License

This is an academic project developed for educational purposes as part of the Operating Systems course curriculum.
