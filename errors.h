#ifndef _ERRORS_H
#define _ERRORS_H

#define SUCCESS 1

// This value is used to indicate that a function
// could not perform its duty because it could either not
// decode some data or it could not make sense of the data
#define DECODING_ERROR -2

// This error code indicates that a function failed
// because of a memory allocation error
#define MEMORY_ERROR -3

// When the image path cannot be loaded
#define CANNOT_LOAD_IMAGE -4

#endif
