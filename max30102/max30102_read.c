#include "max30102_read.h"
const char *iio_dir = "/sys/bus/iio/devices/";

int calc_digits(int num)
{
	int count = 0;
	if (!num) {
		return 1;
	}
	while (num != 0) {
		num /= 10;
		count++;
	}
	return count;
}
static void error_close_dir(DIR *dp)
{
	if (closedir(dp) == -1) {
		perror("find_type_by_name(): Failed to close directory");
	}
}

// get device number by the instance name
// returns device number of matched iio device on success, or an error code on failure
int find_type_by_name(const char *name, const char *type)
{
	const struct dirent *ent;
	FILE *namefp;
	DIR *dp;
	int number = 0, ret, numstrlen;
	char *filename;
	char thisname[IIO_MAX_NAME_LENGTH];

	dp = opendir(iio_dir);
	if (!dp) {
		fprintf(stderr, "No industrialio devices available\n");
		return -ENODEV;
	}

	while ((ent = readdir(dp)) != NULL) {
		if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0) ||
		    (strlen(ent->d_name) <= strlen(type)) ||
		    (strncmp(ent->d_name, type, strlen(type)) != 0)) {
			continue;
		}
		errno = 0;
		ret = sscanf(ent->d_name + strlen(type), "%d", &number);
		if ((ret < 0) || (ret != 1)) {
			ret = -errno;
			fprintf(stderr, "failed to read or match element number\r\n");
			error_close_dir(dp);
			return ret;
		}
		numstrlen = calc_digits(number);

		// verify next character is not a colon
		if (strncmp(ent->d_name + strlen(type) + numstrlen, ":", 1) == 0) {
			continue;
		}

		filename = malloc(strlen(iio_dir) + strlen(type) + numstrlen + 6);
		if (!filename) {
			fprintf(stderr, "failed to allocate memory for filename");
			error_close_dir(dp);
			return ret;
		}

		ret = sprintf(filename, "%s%s%d/name", iio_dir, type, number);
		if (ret < 0) {
			free(filename);
			error_close_dir(dp);
			return ret;
		}

		namefp = fopen(filename, "r");
		free(filename);
		if (!namefp) {
			continue;
		}

		errno = 0;
		if (fscanf(namefp, "%s", thisname) != 1) {
			ret = errno ? -errno : -ENODATA;
			fprintf(stderr, "fscanf for device name failed");
			error_close_dir(dp);
			return ret;
		}
		if (fclose(namefp)) {
			ret = -errno;
			fprintf(stderr, "fclose failed");
			error_close_dir(dp);
			return ret;
		}

		if (strcmp(name, thisname) == 0) {
			if (closedir(dp) == -1) {
				fprintf(stderr, "closedir failed after strcmp of names\r\n");
				return -errno;
			}
			return number;
		}
	}
	if (closedir(dp) == -1) {
		fprintf(stderr, "closedir failed at very very end\r\n");
		return -errno;
	}
	return -ENODEV;
}

int find_data_by_channel_name(const char *channel_name, const char *device_path, int *index,
			      char *type)
{
	FILE *index_fp;
	FILE *type_fp;
	char *index_filename;
	char *type_filename;
	index_filename = malloc(strlen(device_path) + strlen(channel_name) +
				22); // "/scan_elements/ + index + \0" = 21 chars
	type_filename = malloc(strlen(device_path) + strlen(channel_name) +
			       21); // "/scan_elements/ + type + \0" = 20 chars
	sprintf(index_filename, "%s/scan_elements/%s_index", device_path, channel_name);
	sprintf(type_filename, "%s/scan_elements/%s_type", device_path, channel_name);

	index_fp = fopen(index_filename, "r");
	if (index_fp == NULL) {
		fprintf(stderr, "Error opening index_fp: %s. index_filename: %s\r\n",
			strerror(errno), index_filename);
		return -1;
	}

	type_fp = fopen(type_filename, "r");
	if (type_fp == NULL) {
		fprintf(stderr, "Error opening type_fp: %s\r\n", strerror(errno));
		return -1;
	}

	// there should be error checking here, but for ease of time I won't implement it
	fscanf(index_fp, "%d", index);
	fscanf(type_fp, "%s", type);
	fclose(index_fp);
	fclose(type_fp);
	return 0;
}

// parse type string and update iio type struct fields
int decode_type_string(const char *type_string, struct iio_type *type_struct)
{
	char endian_char;
	char format_char;
	// example type string: be:u18/32>>8
	sscanf(type_string, "%ce:%c%u/%u>>%u", &endian_char, &format_char, &type_struct->num_real_bits, &type_struct->num_storage_bits, &type_struct->shift);

	type_struct->is_be = (endian_char == 'b') ? 1 : 0;
	type_struct->is_signed = (format_char == 'u') ? 0 : 1;
	return 0;
}

// hardcoded /dev/iio:device0 path, so if it isn't device0 this won't work. Consider editing later.
int max30102_read_raw_data(const char *device_path, uint32_t *raw_red_data, uint32_t *raw_ir_data) {
	uint8_t record[8];
	FILE *red_en_fp;
	FILE *ir_en_fp;
	FILE *buf_len_fp;
	FILE *buf_en_fp;
	int dev_fd;
	char *red_en_filename = malloc(strlen(device_path) + 35); // "/scan_elements/in_intensity_red_en + \0" = 35 
	char *ir_en_filename = malloc(strlen(device_path) + 34); // "/scan_elements/in_intensity_ir_en + \0" = 34 
	char *buf_len_filename = malloc(strlen(device_path) + 16); // "/buffer0/length + \0" = 16 
	char *buf_en_filename = malloc(strlen(device_path) + 16); // "/buffer0/enable + \0" = 16
	char *dev_filename = malloc(18 * sizeof(char)); // "/dev/iio:device0 + \0" = 17 but make it 18 incase devicenumber is 2 digits
	sprintf(red_en_filename, "%s/scan_elements/in_intensity_red_en", device_path);
	sprintf(ir_en_filename, "%s/scan_elements/in_intensity_ir_en", device_path);
	sprintf(buf_len_filename, "%s/buffer0/length", device_path);
	sprintf(buf_en_filename, "%s/buffer0/enable", device_path);
	sprintf(dev_filename, "/dev/iio:device0"); // NOTE: THIS IS HARDCODED

	red_en_fp = fopen(red_en_filename, "w");
	if (red_en_fp == NULL) {
		fprintf(stderr, "Error opening %s: %s \r\n", red_en_filename, strerror(errno));
		return -1;
	}
	fputc('1', red_en_fp);
	fclose(red_en_fp);

	ir_en_fp = fopen(ir_en_filename, "w");
	if (ir_en_fp == NULL) {
		fprintf(stderr, "Error opening %s: %s \r\n", ir_en_filename, strerror(errno));
		return -1;
	}
	fputc('1', ir_en_fp);
	fclose(ir_en_fp);

	buf_len_fp = fopen(buf_len_filename, "w");
	if (buf_len_fp == NULL) {
		fprintf(stderr, "Error opening %s: %s \r\n", buf_len_filename, strerror(errno));
		return -1;
	}
	fprintf(buf_len_fp, "%d", 128);
	fclose(buf_len_fp);

	buf_en_fp = fopen(buf_en_filename, "w");
	if (buf_en_fp == NULL) {
		fprintf(stderr, "Error opening %s: %s \r\n", buf_en_filename, strerror(errno));
		return -1;
	}
	fputc('1', buf_en_fp);
	fclose(buf_en_fp);

	dev_fd = open(dev_filename, O_RDONLY);
	if (dev_fd < 0) {
		fprintf(stderr, "Error opening %s: %s \r\n", dev_filename, strerror(errno));
		return -1;
	}


	if (read(dev_fd, record, sizeof(record)) != sizeof(record)) {
		fprintf(stderr, "Short read or possible error when reading %s: %s\r\n", dev_filename, strerror(errno));
		return -1;
	}

	*raw_red_data = ((uint32_t)record[0] << 24) | ((uint32_t)record[1] << 16) | ((uint32_t)record[2] << 8) | record[3];
	*raw_ir_data = ((uint32_t)record[4] << 24) | ((uint32_t)record[5] << 16) | ((uint32_t)record[6] << 8) | record[7];

	buf_en_fp = fopen(buf_en_filename, "w");
	if (buf_en_fp == NULL) {
		fprintf(stderr, "Error opening %s: %s \r\n", buf_en_filename, strerror(errno));
		return -1;
	}
	fputc('0', buf_en_fp);
	fclose(buf_en_fp);



	return 0;
}
