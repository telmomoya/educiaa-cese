/* FFT settings */
#define SAMPLES					512 			/* 256 real party and 256 imaginary parts */
#define FFT_SIZE				SAMPLES / 2		/* FFT size is always the same size as we have samples, so 256 in our case */


#define MAX_NGRAN		2		/* max granules */
#define MAX_NCHAN		2		/* max channels */
#define MAX_NSAMP		576		/* max samples per channel, per granule (576 default) */
#define DECODEBUF_SIZE  MAX_NCHAN * MAX_NGRAN * MAX_NSAMP
