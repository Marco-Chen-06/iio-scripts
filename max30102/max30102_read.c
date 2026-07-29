#include "max30102_read.h"
const char *iio_dir = "/sys/bus/iio/devices/";

// get device number by the instance name
// returns device number of matched iio device on success, or an error code on failure
int find_type_by_name(const char *name, const char *type) {
	const struct dirent *ent;
	DIR *dp;
	int number, ret;

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
			if (closedir(dp) == -1) {
				perror("failed to close directory");
			}
			return ret;
		}

	}
	closedir(dp);
	return 0;
}