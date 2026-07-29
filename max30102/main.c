#include "max30102_read.h"

const char *name = "max30102";
const char *type = "iio_device";

int main() {
	int num = find_type_by_name(name, type);
	return 0;
}