#define PROGRAM "test-options"

#include <stdint.h>

#include "../common.h"
#include "../common_options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void test_free_options_accepts_null(void) {
	free_options(NULL);
	ASSERT_TRUE(1);
}

int main(void) {
	test_parse_options_defaults_padding_byte();
	test_parse_options_accepts_hex_padding_byte();
	test_free_options_accepts_null();
	return EXIT_SUCCESS;
}
