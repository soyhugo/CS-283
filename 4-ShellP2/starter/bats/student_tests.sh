#!/usr/bin/env bats

@test "External command: ls runs without errors" {
    run ./dsh <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Built-in cd with no argument does nothing" {
    run ./dsh <<EOF
pwd
cd
pwd
exit
EOF
    # Check that the two pwd outputs (first and second) are the same.
    first_pwd=$(echo "$output" | sed -n '1p')
    second_pwd=$(echo "$output" | sed -n '2p')
    [ "$first_pwd" = "$second_pwd" ]
}

@test "Built-in cd changes directory" {
    run ./dsh <<EOF
mkdir testdir
cd testdir
pwd
exit
EOF
    # The pwd output should include 'testdir'
    echo "$output" | grep "testdir"
    [ "$status" -eq 0 ]
}

@test "External command: echo preserves spaces inside quotes" {
    run ./dsh <<EOF
echo "hello,      world"
exit
EOF
    echo "$output" | grep "hello,      world"
    [ "$status" -eq 0 ]
}

@test "Built-in rc prints not implemented" {
    run ./dsh <<EOF
rc
exit
EOF
    echo "$output" | grep "rc not implemented"
    [ "$status" -eq 0 ]
}
