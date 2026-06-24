#include "../common.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ASSERT_EQ_SIZE(expected, actual) do { \
	size_t expected_ = (expected); \
	size_t actual_ = (actual); \
	if (expected_ != actual_) { \
		fprintf(stderr, "%s:%d: expected %zu, got %zu\n", __FILE__, __LINE__, expected_, actual_); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

static void test_fsize_reports_file_size(void) {
	char filename[] = "/tmp/imgify-test-common-XXXXXX";
	int fd = mkstemp(filename);
	if (fd == -1) {
		perror("mkstemp");
		exit(EXIT_FAILURE);
	}

	const char data[] = "imgify";
	if (write(fd, data, strlen(data)) != (ssize_t) strlen(data)) {
		perror("write");
		close(fd);
		unlink(filename);
		exit(EXIT_FAILURE);
	}
	close(fd);

	ASSERT_EQ_SIZE(strlen(data), fsize(filename));
	unlink(filename);
}

static void test_fsize_exits_for_missing_file(void) {
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull != -1) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
		(void) fsize("/tmp/imgify-test-common-missing-file");
		exit(EXIT_SUCCESS);
	}

	int status;
	if (waitpid(pid, &status, 0) == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_FAILURE) {
		fprintf(stderr, "%s:%d: fsize should exit with failure for missing files\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
}

int main(void) {
	test_fsize_reports_file_size();
	test_fsize_exits_for_missing_file();
	return EXIT_SUCCESS;
}
