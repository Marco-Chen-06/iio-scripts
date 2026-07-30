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
	int number, ret, numstrlen;
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

int find_data_by_channel_name(const char *channel_name, const char *device_path, int *index, char *type) {
	FILE *index_fp;
	FILE *type_fp;
	char *index_filename;
	char *type_filename;
	index_filename = malloc(strlen(device_path) + strlen(channel_name) + 22); // "/scan_elements/ + index + \n" = 21 chars
	type_filename = malloc(strlen(device_path) + strlen(channel_name) + 21); // "/scan_elements/ + type + \n" = 20 chars
	sprintf(index_filename, "%s/scan_elements/%s_index", device_path, channel_name);
	sprintf(type_filename, "%s/scan_elements/%s_type", device_path, channel_name);

	index_fp = fopen(index_filename, "r");
	if (index_fp == NULL) {
		fprintf(stderr, "Error opening index_fp: %s. index_filename: %s\r\n", strerror(errno), index_filename);
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

int decode_type_string(const char *type_string, struct iio_type *type_struct) {
	
}
