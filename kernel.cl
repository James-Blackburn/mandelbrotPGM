__kernel void renderMandelbrot(__global unsigned char* canvas, int PRECISION) {
	double x = ((double)get_global_id(0) / (double)get_global_size(0)) * 4.0 - 2.0;
	double y = ((double)get_global_id(1) / (double)get_global_size(1)) * 4.0 - 2.0;
	const double INITIAL_X = x;
	const double INITIAL_Y = y;

	double a, b;
	int i = 0;
	for (; i < PRECISION; i++) {
		a = x * x - y * y;
		b = 2.0 * x * y;
		x = INITIAL_X + a;
		y = INITIAL_Y + b;

		if (x * x + y * y > 4.0) {
			break;
		}
	}

	if (i < PRECISION) {
		double distance = (double)i / (double)PRECISION;
		canvas[get_global_id(1) * get_global_size(0) + get_global_id(0)] = distance * 255.999;
	}
}