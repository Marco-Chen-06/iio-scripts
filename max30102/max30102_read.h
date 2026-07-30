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

struct iio_type {
    int is_be; // 1 = big endian, 0 = little endian
    int is_signed; // 1 = signed, 0 = unsigned
    int num_real_bits;
    int num_storage_bits;
    int shift;
};

int calc_digits(int num);
// get device number by the instance name
// returns device number of matched iio device on success, or an error code on failure
int find_type_by_name(const char *name, const char *type);

// get index and type of a channel by the channel name
int find_data_by_channel_name(const char *channel_name, const char *device_path, int *index, char *type);

// decode type string attributes and print it
int decode_type_string(const char *type_string, struct iio_type *type_struct);


#endif
