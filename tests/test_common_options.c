#define PROGRAM "test-options"

#include <errno.h>
#include <stdint.h>

#include "../common.h"
#include "../common_options.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ASSERT_TRUE(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #condition); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

#define ASSERT_EQ_U8(expected, actual) do { \
	uint8_t expected_ = (expected); \
	uint8_t actual_ = (actual); \
	if (expected_ != actual_) { \
		fprintf(stderr, "%s:%d: expected 0x%02x, got 0x%02x\n", __FILE__, __LINE__, expected_, actual_); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

#define ASSERT_STREQ(expected, actual) do { \
	const char *expected_ = (expected); \
	const char *actual_ = (actual); \
	if (strcmp(expected_, actual_) != 0) { \
		fprintf(stderr, "%s:%d: expected \"%s\", got \"%s\"\n", __FILE__, __LINE__, expected_, actual_); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

static void reset_getopt(void) {
	optind = 1;
}

static void test_parse_options_defaults_padding_byte(void) {
	char *argv[] = { "test-options", "-i", "input.bin", "-o", "output.png", NULL };
	int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;

	reset_getopt();
	options_t *options = parse_options(argc, argv);

	ASSERT_STREQ("input.bin", options->input);
	ASSERT_STREQ("output.png", options->output);
	ASSERT_EQ_U8(0xff, options->pad_byte);

	free_options(options);
}

static void test_parse_options_accepts_hex_padding_byte(void) {
	char *argv[] = { "test-options", "--input", "input.bin", "--output", "output.png", "--pad_byte", "0x2a", NULL };
	int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;

	reset_getopt();
	options_t *options = parse_options(argc, argv);

	ASSERT_STREQ("input.bin", options->input);
	ASSERT_STREQ("output.png", options->output);
	ASSERT_EQ_U8(0x2a, options->pad_byte);

	free_options(options);
}

static void test_parse_options_accepts_decimal_padding_byte(void) {
	char *argv[] = { "test-options", "-i", "input.bin", "-o", "output.png", "-p", "42", NULL };
	int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;

	reset_getopt();
	options_t *options = parse_options(argc, argv);

	ASSERT_STREQ("input.bin", options->input);
	ASSERT_STREQ("output.png", options->output);
	ASSERT_EQ_U8(42, options->pad_byte);

	free_options(options);
}

static void test_parse_options_accepts_max_padding_byte(void) {
	char *argv[] = { "test-options", "-i", "input.bin", "-o", "output.png", "-p", "255", NULL };
	int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;

	reset_getopt();
	options_t *options = parse_options(argc, argv);

	ASSERT_EQ_U8(0xff, options->pad_byte);

	free_options(options);
}

static void assert_parse_options_exits(char *argv[], int expected_exit_code) {
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull != -1) {
			dup2(devnull, STDOUT_FILENO);
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
		int argc = 0;
		while (argv[argc] != NULL)
			argc++;
		reset_getopt();
		options_t *options = parse_options(argc, argv);
		free_options(options);
		exit(EXIT_SUCCESS);
	}

	int status;
	if (waitpid(pid, &status, 0) == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != expected_exit_code) {
		fprintf(stderr, "%s:%d: parse_options should exit with %d\n", __FILE__, __LINE__, expected_exit_code);
		exit(EXIT_FAILURE);
	}
}

static void assert_parse_options_exits_failure(char *argv[]) {
	assert_parse_options_exits(argv, EXIT_FAILURE);
}

static void test_parse_options_allows_missing_required_values_for_main_to_validate(void) {
	char *argv[] = { "test-options", "-i", "input.bin", NULL };
	int argc = (int) (sizeof(argv) / sizeof(argv[0])) - 1;

	reset_getopt();
	options_t *options = parse_options(argc, argv);

	ASSERT_STREQ("input.bin", options->input);
	ASSERT_TRUE(options->output == NULL);
	ASSERT_EQ_U8(0xff, options->pad_byte);

	free_options(options);
}

static void test_parse_options_rejects_padding_byte_above_byte_range(void) {
	char *argv[] = { "test-options", "-i", "input.bin", "-o", "output.png", "-p", "256", NULL };
	assert_parse_options_exits_failure(argv);
}

static void test_parse_options_rejects_unknown_option(void) {
	char *argv[] = { "test-options", "--unknown-option", NULL };
	assert_parse_options_exits_failure(argv);
}

static void test_parse_options_help_exits_success(void) {
	char *argv[] = { "test-options", "--help", NULL };
	assert_parse_options_exits(argv, EXIT_SUCCESS);
}

static void test_parse_options_version_exits_success(void) {
	char *argv[] = { "test-options", "-v", NULL };
	assert_parse_options_exits(argv, EXIT_SUCCESS);
}

static void test_free_options_accepts_null(void) {
	free_options(NULL);
	ASSERT_TRUE(1);
}

int main(void) {
	test_parse_options_defaults_padding_byte();
	test_parse_options_accepts_hex_padding_byte();
	test_parse_options_accepts_decimal_padding_byte();
	test_parse_options_accepts_max_padding_byte();
	test_parse_options_allows_missing_required_values_for_main_to_validate();
	test_parse_options_rejects_padding_byte_above_byte_range();
	test_parse_options_rejects_unknown_option();
	test_parse_options_help_exits_success();
	test_parse_options_version_exits_success();
	test_free_options_accepts_null();
	return EXIT_SUCCESS;
}
