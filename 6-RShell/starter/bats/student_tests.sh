#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Check pwd runs correctly" {
    run ./dsh <<EOF
pwd
EOF
    expected_output="$(pwd)"
    actual_output="$(echo "$output" | head -n 1 | tr -d '\r\n')"
    [ "$status" -eq 0 ]
    [[ "$actual_output" == "$expected_output" ]]
}

@test "Check echo prints the correct message" {
    run ./dsh <<EOF
echo "Hello, world!"
EOF
    expected_output="Hello, world!"
    actual_output="$(echo "$output" | head -n 1 | tr -d '\r\n')"
    [ "$status" -eq 0 ]
    [[ "$actual_output" == "$expected_output" ]]
}

@test "Check cd changes directory" {
    run ./dsh <<EOF
cd ..
pwd
EOF
    expected_output="$(cd .. && pwd)"
    actual_output="$(echo "$output" | grep -Eo '/mnt/c/Users/hugol/Desktop/Drexel courses/cs283/CS-283/6-RShell')"
    echo "Expected: $expected_output"
    echo "Actual: $actual_output"
    [ "$status" -eq 0 ]
    [[ "$actual_output" == "$expected_output" ]]
}

@test "Check exit command works" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Check basic piping works" {
    run ./dsh <<EOF
ls | wc -l
EOF
    trimmed_output="$(echo "$output" | head -n 1 | tr -d '[:space:]')"
    [ "$status" -eq 0 ]
    [[ "$trimmed_output" =~ ^[0-9]+$ ]]
}

@test "Check built-in command exit works" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

