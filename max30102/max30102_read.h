#ifndef MAX30102_READ_H
#define MAX30102_READ_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

#define IIO_MAX_NAME_LENGTH 256
extern const char *iio_dir;

int calc_digits(int num);
// get device number by the instance name
// returns device number of matched iio device on success, or an error code on failure
int find_type_by_name(const char *name, const char *type);

// get index and type of a channel by the channel name
int find_data_by_channel_name(const char *channel_name, const char *device_path, int *index, char *type);


#endif