#include "../common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main(void) {
	test_fsize_reports_file_size();
	return EXIT_SUCCESS;
}
