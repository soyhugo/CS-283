1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: Using fork creates a child process that runs the new program, allowing the parent shell to continue running. This separation prevents the shell from being replaced by the new program and lets the shell manage multiple commands.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: If fork() fails, it returns -1. In our implementation, we check for this error, print an error message with perror, and return an error code, ensuring the shell doesn't attempt further execution in an invalid state.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: execvp() searches the directories listed in the PATH environment variable to locate the executable file corresponding to the command.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**: wait() makes the parent process pause until the child process completes, which prevents zombie processes. Without wait(), the parent might continue before the child finishes, potentially leading to resource leaks.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() extracts the exit status of the child process from the status returned by wait(). This exit status indicates whether the command executed successfully or encountered an error, which is crucial for handling outcomes properly.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: build_cmd_buff() treats everything between a pair of double quotes as a single token, preserving spaces within. This is necessary so that arguments containing spaces are not split into separate tokens.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: In the previous assignment, my parsing logic was based on build_cmd_list(), which split the command line into multiple commands using pipes and then tokenized each command by spaces. For this assignment, I switched to using a single cmd_buff structure (build_cmd_buff()) that doesn’t handle pipes at all. I also added support for quoted arguments to preserve spaces within quotes, and simplified the trimming of extra spaces. The challenge was ensuring that quoted strings were handled correctly without breaking the overall tokenization process.

8. For this question, you need to do some research on Linux signals.
- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: Signals provide asynchronous notifications to processes, alerting them to events such as interrupts or termination requests. Unlike other IPC mechanisms that involve explicit data exchange, signals act like interrupts that can preempt normal processing.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: 
      - SIGKILL: Forces an immediate termination of a process; it cannot be caught or ignored, used for killing unresponsive processes.
      - SIGTERM: Requests a graceful termination, allowing processes to perform cleanup before exiting.
      - SIGINT: Sent when a user interrupts a process (e.g., by pressing Ctrl-C), allowing the process to handle or terminate based on the interrupt.
      
- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: When a process receives SIGSTOP, it is immediately suspended by the kernel. SIGSTOP cannot be caught or ignored, unlike SIGINT, because it is a non-maskable signal designed to ensure that the process is reliably stopped.
