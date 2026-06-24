#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../imgify.h"

#define ASSERT_TRUE(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #condition); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ_U32(expected, actual) do { \
	uint32_t expected_ = (expected); \
	uint32_t actual_ = (actual); \
	if (expected_ != actual_) { \
		fprintf(stderr, "%s:%d: expected %u, got %u\n", __FILE__, __LINE__, expected_, actual_); \
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

#define ASSERT_EQ_SIZE(expected, actual) do { \
	size_t expected_ = (expected); \
	size_t actual_ = (actual); \
	if (expected_ != actual_) { \
		fprintf(stderr, "%s:%d: expected %zu, got %zu\n", __FILE__, __LINE__, expected_, actual_); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

static void make_temp_filename(char *filename) {
	int fd = mkstemp(filename);
	if (fd == -1) {
		perror("mkstemp");
		exit(EXIT_FAILURE);
	}
	close(fd);
}

static void test_png_round_trip_preserves_original_size_and_padding(void) {
	char filename[] = "/tmp/imgify-test-imgify-XXXXXX";
	make_temp_filename(filename);

	uint8_t input[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };
	const size_t original_size = sizeof(input);
	const uint32_t width = 2;
	const uint32_t height = 1;
	const uint8_t channels = 4;
	const uint32_t padding = width * height * channels - original_size;
	const uint8_t pad_byte = 0xab;

	ASSERT_TRUE(png_save(filename, input, width, height, channels, padding, pad_byte, original_size));

	uint8_t *output = NULL;
	size_t output_size = 0;
	uint32_t output_width = 0;
	uint32_t output_height = 0;
	uint8_t output_channels = 0;
	uint32_t output_padding = 0;

	ASSERT_TRUE(png_load(filename, &output, &output_size, &output_width, &output_height, &output_channels, &output_padding));

	ASSERT_EQ_SIZE(original_size, output_size);
	ASSERT_EQ_U32(width, output_width);
	ASSERT_EQ_U32(height, output_height);
	ASSERT_EQ_U8(channels, output_channels);
	ASSERT_EQ_U32(padding, output_padding);
	ASSERT_TRUE(memcmp(input, output, original_size) == 0);
	for (uint32_t i = 0; i < padding; ++i) {
		ASSERT_EQ_U8(pad_byte, output[original_size + i]);
	}

	free(output);
	unlink(filename);
}

static void test_png_round_trip_without_padding_uses_full_image_size(void) {
	char filename[] = "/tmp/imgify-test-imgify-XXXXXX";
	make_temp_filename(filename);

	uint8_t input[] = { 0x10, 0x11, 0x12, 0x13 };
	const size_t original_size = sizeof(input);

	ASSERT_TRUE(png_save(filename, input, 1, 1, 4, 0, 0xff, original_size));

	uint8_t *output = NULL;
	size_t output_size = 0;
	uint32_t output_padding = 1;

	ASSERT_TRUE(png_load(filename, &output, &output_size, NULL, NULL, NULL, &output_padding));

	ASSERT_EQ_SIZE(original_size, output_size);
	ASSERT_EQ_U32(0, output_padding);
	ASSERT_TRUE(memcmp(input, output, original_size) == 0);

	free(output);
	unlink(filename);
}

static void test_png_round_trip_rgb_without_padding(void) {
	char filename[] = "/tmp/imgify-test-imgify-XXXXXX";
	make_temp_filename(filename);

	uint8_t input[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04 };
	const size_t original_size = sizeof(input);

	ASSERT_TRUE(png_save(filename, input, 2, 1, 3, 0, 0xff, original_size));

	uint8_t *output = NULL;
	size_t output_size = 0;
	uint32_t output_width = 0;
	uint32_t output_height = 0;
	uint8_t output_channels = 0;
	uint32_t output_padding = 99;

	ASSERT_TRUE(png_load(filename, &output, &output_size, &output_width, &output_height, &output_channels, &output_padding));

	ASSERT_EQ_SIZE(original_size, output_size);
	ASSERT_EQ_U32(2, output_width);
	ASSERT_EQ_U32(1, output_height);
	ASSERT_EQ_U8(3, output_channels);
	ASSERT_EQ_U32(0, output_padding);
	ASSERT_TRUE(memcmp(input, output, original_size) == 0);

	free(output);
	unlink(filename);
}

static void test_png_load_rejects_non_png_file(void) {
	char filename[] = "/tmp/imgify-test-imgify-XXXXXX";
	make_temp_filename(filename);

	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	fputs("not a png", fp);
	fclose(fp);

	uint8_t *output = NULL;
	size_t output_size = 123;

	int saved_stderr = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	if (saved_stderr == -1 || devnull == -1 || dup2(devnull, STDERR_FILENO) == -1) {
		perror("redirect stderr");
		exit(EXIT_FAILURE);
	}

	bool ok = png_load(filename, &output, &output_size, NULL, NULL, NULL, NULL);

	if (dup2(saved_stderr, STDERR_FILENO) == -1) {
		perror("restore stderr");
		exit(EXIT_FAILURE);
	}
	close(saved_stderr);
	close(devnull);

	ASSERT_FALSE(ok);
	ASSERT_TRUE(output == NULL);
	ASSERT_EQ_SIZE(123, output_size);

	unlink(filename);
}

static void test_png_save_rejects_unknown_channel_count(void) {
	char filename[] = "/tmp/imgify-test-imgify-XXXXXX";
	make_temp_filename(filename);

	uint8_t input[] = { 0x00, 0x01 };

	int saved_stderr = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	if (saved_stderr == -1 || devnull == -1 || dup2(devnull, STDERR_FILENO) == -1) {
		perror("redirect stderr");
		exit(EXIT_FAILURE);
	}

	bool ok = png_save(filename, input, 1, 1, 2, 0, 0xff, sizeof(input));

	if (dup2(saved_stderr, STDERR_FILENO) == -1) {
		perror("restore stderr");
		exit(EXIT_FAILURE);
	}
	close(saved_stderr);
	close(devnull);

	ASSERT_FALSE(ok);

	unlink(filename);
}

int main(void) {
	test_png_round_trip_preserves_original_size_and_padding();
	test_png_round_trip_without_padding_uses_full_image_size();
	test_png_round_trip_rgb_without_padding();
	test_png_load_rejects_non_png_file();
	test_png_save_rejects_unknown_channel_count();
	return EXIT_SUCCESS;
}
