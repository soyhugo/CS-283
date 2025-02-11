1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

   > **Answer**:  `fgets()` is a good choice because it reads input one line at a time, which fits perfectly with the shell’s requirement to process commands line-by-line. It also allows you to specify a maximum number of characters to read, protecting against buffer overflow, and it automatically appends a null terminator to the string, ensuring that the input is well-formed for further processing.

2. You needed to use `malloc()` to allocate memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

   > **Answer**:  Using `malloc()` allows dynamic allocation of memory for `cmd_buff`, which can handle varying sizes of input more flexibly. This means you can adjust the buffer size at runtime depending on the expected input length, thus avoiding potential issues of wasted memory or truncation of input if a fixed-size array were used. However, in an assignment where a maximum command length is defined, a fixed-size array might be sufficient—but `malloc()` offers greater scalability for different use cases.

3. In `dshlib.c`, the function `build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

   > **Answer**:  Trimming spaces is necessary to ensure that the commands and their arguments are parsed accurately. Without trimming, extra spaces could be interpreted as additional empty tokens or part of the command name/arguments, leading to execution errors or misinterpretation by the shell. This could cause commands to fail or behave unexpectedly, as they might not match the intended format.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

   - One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

     > **Answer**:  
     > **Example 1: STDOUT Redirection (`command > file.txt`)** – This redirects the command’s standard output to a file. The challenge is ensuring that the file is opened (or created) with the correct permissions and that the STDOUT file descriptor is correctly replaced.  
     > **Example 2: STDIN Redirection (`command < file.txt`)** – This takes input for a command from a file instead of the keyboard. The challenge here is to properly open the file for reading and substitute the STDIN file descriptor, while handling cases where the file might not exist.  
     > **Example 3: STDERR Redirection (`command 2> error.txt`)** – This directs error output to a separate file. The challenge is managing the STDERR file descriptor independently so that error messages are captured accurately without interfering with the normal output.

   - You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

     > **Answer**:  Redirection involves changing the source or destination of the input or output of a command, typically to or from a file or device. Piping, on the other hand, connects the output of one command directly as the input of another, facilitating real-time data streaming between processes. Essentially, redirection deals with persistent data storage or retrieval, while piping is about dynamic inter-process communication.

   - STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

     > **Answer**:  Keeping STDERR and STDOUT separate is important because it allows users and programs to clearly distinguish between normal output and error messages. This separation is crucial for debugging, logging, and filtering purposes, as it enables error messages to be captured or redirected independently of standard output, thereby enhancing clarity and usability.

   - How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

     > **Answer**:  Our custom shell should, by default, keep STDOUT and STDERR separate to preserve the clarity of output and error messages. However, it should also support an option to merge these streams—typically via a redirection operator such as `2>&1`—when the user explicitly requests it. This allows for flexible error handling, enabling users to combine the outputs for logging or debugging when necessary.
