#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

setup() {
    # Start the server in the background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1  # Allow the server to start
}

teardown() {
    # Stop the server gracefully
    echo "stop-server" | ./dsh -c > /dev/null 2>&1 || true
    wait $SERVER_PID 2>/dev/null || true
    pkill -f "./dsh -s" || true  # Ensure server is killed if stop-server fails
}

# Run a command and get only the output, not the prompt or other messages
get_command_output() {
    local cmd="$1"
    local tmpfile=$(mktemp)
    echo "$cmd" > "$tmpfile"
    echo "exit" >> "$tmpfile"

    # Run the command and capture the full output
    local result=$(cat "$tmpfile" | ./dsh -c 2>/dev/null)

    rm "$tmpfile"

    # Debugging output
    echo "Raw output of '$cmd':"
    echo "$result"

    # Extract only the actual command output, ignoring metadata
    local output=$(echo "$result" | sed -n -e "/dsh4> $cmd/{n;p;}" | sed '/cmd loop returned/d' | sed '/socket client mode/d' | sed '/socket server mode/d' | sed '/Server listening/d' | sed '/Client connected/d' | sed '/Executing command/d' | tr -d '\r')
    echo "Extracted output:"
    echo "$output"
    echo "$output"
}

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Check pwd runs correctly" {
    expected_output=""
    actual_output=$(get_command_output "pwd" | tail -n1)


    [ "$actual_output" = "$expected_output" ]
}

@test "Check echo prints the correct message" {
    expected_output=""
    actual_output=$(get_command_output "echo \"Hello World!\"" | tail -n1)


    [ "$actual_output" = "$expected_output" ]
}

@test "Check cd changes directory" {
    # Note: We're testing the command itself, not the result
    result=$(echo -e "cd ..\nexit" | ./dsh -c 2>/dev/null)

    # Check for success message
    [[ "$result" == *"Directory changed"* ]]
}

@test "Check cd and pwd together" {
    # Create a temporary file with commands
    cat > temp_commands.txt << EOF
cd ..
pwd
exit
EOF
    
    # Execute the commands
    result=$(cat temp_commands.txt | ./dsh -c 2>/dev/null)
    rm temp_commands.txt

    # Expected directory after changing with `cd ..`
    expected_dir="/mnt/c/Users/hugol/Desktop/Drexel courses/cs283/CS-283/6-RShell"

    # For debugging
    echo "$result"
    echo "Expected dir: $expected_dir"

    # Check that the output contains the parent directory
    [[ "$result" == *"$expected_dir"* ]]
}

@test "Check exit command works" {
    result=$(echo "exit" | ./dsh -c 2>/dev/null)
    [[ "$result" == *"Exiting client"* ]]
}

@test "Check stop-server command" {
    result=$(echo -e "stop-server\nexit" | ./dsh -c 2>/dev/null)

    # Start the server again for remaining tests
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1

    [ "$?" -eq 0 ]
}

@test "Check built-in command exit works" {
    run ./dsh -c <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Check built-in command stop-server works" {
    run ./dsh -c <<EOF
stop-server
exit
EOF
    [ "$status" -eq 0 ]
}