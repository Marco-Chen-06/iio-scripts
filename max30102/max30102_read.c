#include "max30102_read.h"
const char *iio_dir = "/sys/bus/iio/devices/";

static int calc_digits(int num)
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