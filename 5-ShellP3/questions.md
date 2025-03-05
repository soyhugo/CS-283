1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation waits for all child processes by looping through all forked PIDs and calling waitpid() on each. This ensures that the parent process doesn’t continue accepting input until every child has finished execution. If waitpid() were omitted, child processes could become zombie processes, causing resource leaks and potentially leaving unfinished work running in the background.


2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

After using dup2() to redirect a file descriptor, the original pipe ends remain open unless explicitly closed. Leaving them open can cause problems such as preventing the pipe from sending an EOF to the reading end, leading to blocking behavior. It also wastes file descriptors, which may eventually exhaust system resources.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command is implemented as a built-in because it changes the shell’s current working directory—a property of the shell process itself. If cd were external, it would run in a separate child process, and any directory change would affect only that process, leaving the parent shell’s directory unchanged. This would make the command ineffective for the user’s session.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To allow an arbitrary number of piped commands, I would replace the fixed-size array (CMD_MAX) with a dynamically allocated data structure, such as a resizable array (using realloc()) or a linked list. The trade-offs include added complexity in memory management and potential performance overhead during resizing, but this approach provides flexibility to handle any number of commands.
