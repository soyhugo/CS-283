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

@test "Single command: ls runs without error" {
    run ./dsh <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Simple pipeline: ls | grep .c" {
    run ./dsh <<EOF
ls | grep .c
exit
EOF
    echo "$output" | grep "dshlib.c"
    [ "$status" -eq 0 ]
}

@test "Pipeline with three commands: ls | grep .c | wc -l" {
    run ./dsh <<EOF
ls | grep .c | wc -l
exit
EOF
    [ "$status" -eq 0 ]
}


@test "Empty input should print CMD_WARN_NO_CMD" {
    run ./dsh <<EOF

exit
EOF
    echo "$output" | grep "warning: no commands provided"
    [ "$status" -eq 0 ]
}

@test "Exit command should terminate the shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}
