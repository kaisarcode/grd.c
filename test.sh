#!/bin/bash
# test.sh
# Summary: Validation suite for grd CLI split behavior.
# Author:  KaisarCode
# Website: https://kaisarcode.com
# License: https://www.gnu.org/licenses/gpl-3.0.html

# Prints one failure line using the shared KCS color style.
# @param 1 Failure message.
# @return 1 on failure.
kc_test_fail() {
    printf "\033[31m[FAIL]\033[0m %s\n" "$1"
    return 1
}

# Prints one success line using the shared KCS color style.
# @param 1 Success message.
# @return 0 on success.
kc_test_pass() {
    printf "\033[32m[PASS]\033[0m %s\n" "$1"
}

# Checks whether the grd binary exists in the current directory.
# @return 0 on success, 1 on failure.
kc_test_check_binary() {
    if [ ! -f "./grd" ]; then
        kc_test_fail "binary ./grd not found"
        return 1
    fi
    return 0
}

# Runs one CLI test case and compares full stdout with the expected value.
# @param 1 Case label.
# @param 2 Expected stdout.
# @param 3... Command and arguments to execute.
# @return 0 on success, 1 on failure.
kc_test_run_case() {
    local label="$1"
    local expected="$2"
    shift 2

    local actual
    actual="$("$@" 2>/dev/null)"
    if [ "$actual" = "$expected" ]; then
        kc_test_pass "$label"
        return 0
    fi

    kc_test_fail "$label"
    printf "  expected: %s\n" "$expected"
    printf "  actual:   %s\n" "$actual"
    return 1
}

# Runs one CLI error test case and checks that exit code is non-zero.
# @param 1 Case label.
# @param 2... Command and arguments to execute.
# @return 0 on success, 1 on failure.
kc_test_run_error_case() {
    local label="$1"
    shift

    if ! "$@" >/dev/null 2>&1; then
        kc_test_pass "$label"
        return 0
    fi

    kc_test_fail "$label"
    return 1
}

# Runs the full validation suite for grd split behavior.
# @return 0 when all tests pass, 1 otherwise.
kc_test_main() {
    local failed=0
    local expected

    kc_test_check_binary || exit 1

    expected="$(cat <<'EOF'
0 0 0 50 100
1 50 0 50 100
EOF
)"
    kc_test_run_case "row equal split" "$expected" \
        ./grd split -w 100 -H 100 -k row -W "1 1" || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 100 50
1 0 50 100 50
EOF
)"
    kc_test_run_case "col equal split" "$expected" \
        ./grd split -w 100 -H 100 -k col -W "1 1" || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 30 60
1 30 0 60 60
2 90 0 30 60
EOF
)"
    kc_test_run_case "row unequal split 1:2:1" "$expected" \
        ./grd split -w 120 -H 60 -k row -W "1 2 1" || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 48 50
1 52 0 48 50
EOF
)"
    kc_test_run_case "row split with gap" "$expected" \
        ./grd split -w 100 -H 50 -k row -W "1 1" -g 4 || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 100 24
1 0 26 100 24
2 0 52 100 48
EOF
)"
    kc_test_run_case "col split with gap" "$expected" \
        ./grd split -w 100 -H 100 -k col -W "1 1 2" -g 2 || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 4 10
1 4 0 3 10
2 7 0 3 10
EOF
)"
    kc_test_run_case "row rounding distributes remainder to first child" "$expected" \
        ./grd split -w 10 -H 10 -k row -W "1 1 1" || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 25 100
1 25 0 75 100
EOF
)"
    kc_test_run_case "row unequal 1:3 weights" "$expected" \
        ./grd split -w 100 -H 100 -k row -W "1 3" || failed=$((failed + 1))

    expected="$(cat <<'EOF'
0 0 0 100 25
1 0 25 100 75
EOF
)"
    kc_test_run_case "col unequal 1:3 weights" "$expected" \
        ./grd split -w 100 -H 100 -k col -W "1 3" || failed=$((failed + 1))

    kc_test_run_case "comma-separated weights" \
        "$(./grd split -w 100 -H 100 -k row -W "1 1" 2>/dev/null)" \
        ./grd split -w 100 -H 100 -k row -W "1,1" || failed=$((failed + 1))

    kc_test_run_case "row is default kind" \
        "$(./grd split -w 100 -H 100 -k row -W "1 2" 2>/dev/null)" \
        ./grd split -w 100 -H 100 -W "1 2" || failed=$((failed + 1))

    kc_test_run_case "version flag exits zero" "grd $( ./grd --version | awk '{print $2}')" \
        ./grd --version || failed=$((failed + 1))

    kc_test_run_error_case "missing --width"  ./grd split -H 100 -W "1 1" || failed=$((failed + 1))
    kc_test_run_error_case "missing --height" ./grd split -w 100 -W "1 1" || failed=$((failed + 1))
    kc_test_run_error_case "missing --weights" ./grd split -w 100 -H 100 || failed=$((failed + 1))
    kc_test_run_error_case "single weight rejected" ./grd split -w 100 -H 100 -W "1" || failed=$((failed + 1))
    kc_test_run_error_case "zero weight rejected" ./grd split -w 100 -H 100 -W "0 1" || failed=$((failed + 1))
    kc_test_run_error_case "negative weight rejected" ./grd split -w 100 -H 100 -W "-1 1" || failed=$((failed + 1))
    kc_test_run_error_case "unknown command rejected" ./grd unknown || failed=$((failed + 1))
    kc_test_run_error_case "unknown flag rejected" ./grd split -w 100 -H 100 -W "1 1" --foo || failed=$((failed + 1))
    kc_test_run_error_case "invalid kind rejected" ./grd split -w 100 -H 100 -W "1 1" -k diag || failed=$((failed + 1))
    kc_test_run_error_case "zero width rejected" ./grd split -w 0 -H 100 -W "1 1" || failed=$((failed + 1))
    kc_test_run_error_case "zero height rejected" ./grd split -w 100 -H 0 -W "1 1" || failed=$((failed + 1))

    if [ "$failed" -eq 0 ]; then
        return 0
    fi

    return 1
}

kc_test_main
