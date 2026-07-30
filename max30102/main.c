#include "max30102_read.h"

int main()
{
	const char *name = "max30102";
	const char *type = "iio:device";
	int number = find_type_by_name(name, type);
	int numstrlen = calc_digits(number);

	// find device pathname and print it
	char *device_path = malloc(strlen(iio_dir) + strlen(type) + numstrlen + 6);
	sprintf(device_path, "%s%s%d", iio_dir, type, number);
	printf("Found device: %s\r\n", device_path);

	// for each channel in <path>/scan_elements, print name, _index value, and _type string
	const char *in_intensity_red_name = "in_intensity_red";
	const char *in_intensity_ir_name = "in_intensity_ir";
	int in_intensity_red_index = 0;
	int in_intensity_ir_index = 0;
	char *in_intensity_red_type =
		malloc(20 * sizeof(char)); // format string usually around 10 characters long
	char *in_intensity_ir_type = malloc(20 * sizeof(char));
	find_data_by_channel_name(in_intensity_red_name, device_path, &in_intensity_red_index,
				  in_intensity_red_type);
	find_data_by_channel_name(in_intensity_ir_name, device_path, &in_intensity_ir_index,
				  in_intensity_ir_type);
	printf("Channel: %s   index=%d  type=%s\r\n", in_intensity_red_name, in_intensity_red_index,
	       in_intensity_red_type);
	printf("Channel: %s   index=%d  type=%s\r\n", in_intensity_ir_name, in_intensity_ir_index,
	       in_intensity_ir_type);

	// decode type string and fill out appropriate fields in type struct
	struct iio_type red_type_struct;
	struct iio_type ir_type_struct;
	decode_type_string(in_intensity_red_type, &red_type_struct);
	decode_type_string(in_intensity_ir_type, &ir_type_struct);
	printf("endian=%s, signed=%s, realbits=%d, storagebits=%d, shift=%d -> bytes_per_channel=%d\r\n",
	       red_type_struct.is_be ? "big" : "little", red_type_struct.is_signed ? "yes" : "no",
	       red_type_struct.num_real_bits, red_type_struct.num_storage_bits,
	       red_type_struct.shift, red_type_struct.num_storage_bits / 8);

	return number;
}
