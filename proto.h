/*
 * Protocol definition file
 */

// UDP-package
typedef struct {
        char timestamp[14];
        char image[65400];
} vpkg;

