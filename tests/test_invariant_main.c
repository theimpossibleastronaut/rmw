#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

START_TEST(test_buffer_overflow_protection)
{
    // Invariant: Buffer reads never exceed declared length; oversized inputs must be rejected or truncated safely
    const char *payloads[] = {
        "valid_file.txt",                                                           // valid input
        "a",                                                                        // boundary: minimal
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", // 256+ chars (overflow attempt)
        "/path/to/file/../../../etc/passwd/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" // path traversal + overflow
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        pid_t pid = fork();
        ck_assert_int_ne(pid, -1);
        
        if (pid == 0) {
            // Child process: run main with oversized argument
            char *argv[] = {"./src/main", "-r", (char *)payloads[i], NULL};
            execv("./src/main", argv);
            exit(127); // exec failed
        } else {
            // Parent: wait for child and verify it doesn't crash with SIGSEGV
            int status;
            waitpid(pid, &status, 0);
            
            // Process should either exit normally or with non-segfault signal
            if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);
                ck_assert_msg(sig != SIGSEGV, "Buffer overflow detected: SIGSEGV on payload %d", i);
            }
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_overflow_protection);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}