#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.gswatcher"), aligned (8)))
#else
# define SECTION
#endif

static const SECTION union { const guint8 data[3376]; const double alignment; void * const ptr;}  gswatcher_resource_data = { {
  0x47, 0x56, 0x61, 0x72, 0x69, 0x61, 0x6e, 0x74, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x18, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x28, 0x06, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
  0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 
  0x87, 0x04, 0xb0, 0x19, 0x03, 0x00, 0x00, 0x00, 
  0xc8, 0x00, 0x00, 0x00, 0x07, 0x00, 0x76, 0x00, 
  0xd0, 0x00, 0x00, 0x00, 0xdf, 0x04, 0x00, 0x00, 
  0x4b, 0x50, 0x90, 0x0b, 0x02, 0x00, 0x00, 0x00, 
  0xdf, 0x04, 0x00, 0x00, 0x04, 0x00, 0x4c, 0x00, 
  0xe4, 0x04, 0x00, 0x00, 0xe8, 0x04, 0x00, 0x00, 
  0xd4, 0xb5, 0x02, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xe8, 0x04, 0x00, 0x00, 0x01, 0x00, 0x4c, 0x00, 
  0xec, 0x04, 0x00, 0x00, 0xf0, 0x04, 0x00, 0x00, 
  0x6f, 0x22, 0x51, 0x95, 0x05, 0x00, 0x00, 0x00, 
  0xf0, 0x04, 0x00, 0x00, 0x03, 0x00, 0x4c, 0x00, 
  0xf4, 0x04, 0x00, 0x00, 0xfc, 0x04, 0x00, 0x00, 
  0xcd, 0xa9, 0x63, 0xbe, 0x03, 0x00, 0x00, 0x00, 
  0xfc, 0x04, 0x00, 0x00, 0x0e, 0x00, 0x76, 0x00, 
  0x10, 0x05, 0x00, 0x00, 0x1f, 0x0d, 0x00, 0x00, 
  0x42, 0x32, 0xc0, 0xd1, 0x01, 0x00, 0x00, 0x00, 
  0x1f, 0x0d, 0x00, 0x00, 0x0a, 0x00, 0x4c, 0x00, 
  0x2c, 0x0d, 0x00, 0x00, 0x30, 0x0d, 0x00, 0x00, 
  0x69, 0x6e, 0x66, 0x6f, 0x2e, 0x75, 0x69, 0x00, 
  0x7c, 0x2a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
  0x78, 0xda, 0xed, 0x5a, 0x6d, 0x6f, 0x9b, 0x30, 
  0x10, 0xfe, 0xde, 0x5f, 0xc1, 0xd8, 0x97, 0x4d, 
  0x13, 0xcd, 0x5b, 0x5f, 0xb2, 0x29, 0xa1, 0x5a, 
  0x27, 0xad, 0x9a, 0xd4, 0x4d, 0xd3, 0xd4, 0xed, 
  0x2b, 0x32, 0xe6, 0x20, 0x5e, 0x0c, 0x66, 0xb6, 
  0xa1, 0xc9, 0x7e, 0xfd, 0x8e, 0x40, 0xd2, 0x34, 
  0xef, 0x4d, 0x37, 0x06, 0x51, 0xf8, 0x44, 0xc4, 
  0x63, 0x73, 0x77, 0xcf, 0x73, 0x67, 0x1f, 0x4e, 
  0xef, 0x6a, 0x14, 0x72, 0x23, 0x05, 0xa9, 0x98, 
  0x88, 0xfa, 0x66, 0xeb, 0xb4, 0x69, 0x1a, 0x10, 
  0x51, 0xe1, 0xb1, 0x28, 0xe8, 0x9b, 0xdf, 0xef, 
  0x3e, 0x5a, 0x5d, 0xf3, 0xca, 0x3e, 0xe9, 0xb1, 
  0x48, 0x83, 0xf4, 0x09, 0x05, 0xbb, 0xf7, 0xc2, 
  0xb2, 0x8c, 0xd9, 0x4f, 0x4b, 0xc2, 0xaf, 0x84, 
  0x49, 0x50, 0x46, 0xa0, 0x87, 0x6f, 0x8c, 0xce, 
  0xe9, 0x85, 0x61, 0x59, 0x76, 0x4f, 0xb8, 0x3f, 
  0x81, 0x6a, 0x83, 0x72, 0xa2, 0x54, 0xdf, 0xbc, 
  0xd1, 0xc3, 0x1b, 0xc9, 0x3c, 0xd3, 0x60, 0x5e, 
  0xdf, 0x8c, 0x49, 0x04, 0xdc, 0xb4, 0x7b, 0xb1, 
  0x14, 0x31, 0x48, 0x3d, 0x36, 0x22, 0x12, 0x42, 
  0xdf, 0x4c, 0x99, 0x62, 0x2e, 0x07, 0xd3, 0xbe, 
  0x93, 0x09, 0xf4, 0x1a, 0xd3, 0xa7, 0x4b, 0x38, 
  0x4a, 0x22, 0xc7, 0x17, 0x34, 0x51, 0xa6, 0xfd, 
  0x91, 0x70, 0xb5, 0x09, 0xea, 0x0a, 0xe9, 0x81, 
  0x74, 0xee, 0x99, 0xa7, 0x07, 0xa6, 0x7d, 0xb6, 
  0x01, 0x29, 0xc5, 0xbd, 0xa3, 0x62, 0x42, 0xd1, 
  0xe9, 0xcd, 0x40, 0x2a, 0x78, 0x12, 0x46, 0x0f, 
  0xd8, 0xee, 0x3c, 0x96, 0x0e, 0x18, 0xf7, 0x56, 
  0xf8, 0x7e, 0x4b, 0x5c, 0xf4, 0x78, 0xe2, 0x3c, 
  0xcf, 0x6e, 0x5b, 0xa5, 0x78, 0x3f, 0x22, 0x9c, 
  0x05, 0x91, 0x69, 0x37, 0x37, 0x60, 0xc6, 0x3b, 
  0x60, 0x78, 0x6e, 0xbd, 0x96, 0x24, 0x52, 0x9c, 
  0x68, 0x82, 0x66, 0xe2, 0x40, 0x40, 0x0b, 0xbe, 
  0xe0, 0xf3, 0x77, 0xf3, 0x23, 0x1b, 0xb9, 0xeb, 
  0x38, 0x05, 0xa1, 0x43, 0x8c, 0xcf, 0xf2, 0x5c, 
  0xe0, 0x6b, 0x87, 0x68, 0x4d, 0xe8, 0x60, 0xf3, 
  0x4b, 0xb5, 0x88, 0x77, 0xc2, 0x15, 0xdc, 0xb6, 
  0x36, 0x40, 0x06, 0xc0, 0x82, 0x81, 0x5e, 0xc0, 
  0x34, 0x66, 0x16, 0x36, 0x0a, 0xd6, 0x76, 0x25, 
  0xaf, 0x7d, 0x20, 0xe4, 0xbd, 0xf7, 0x3c, 0xcc, 
  0x5a, 0x55, 0x02, 0x7f, 0xad, 0x4a, 0xf1, 0xd7, 
  0x39, 0x10, 0xfe, 0x6e, 0xca, 0x49, 0xbe, 0x76, 
  0xa5, 0xc8, 0x3b, 0x3b, 0x10, 0xf2, 0x3e, 0x93, 
  0xb8, 0x04, 0xee, 0x3a, 0x95, 0xe2, 0xee, 0xfc, 
  0x40, 0xb8, 0xfb, 0x2a, 0x59, 0x4a, 0x74, 0x19, 
  0xb9, 0x77, 0x5e, 0x29, 0xfe, 0x2e, 0x0e, 0x85, 
  0x3f, 0x4e, 0xc6, 0xb8, 0xdd, 0x2d, 0x81, 0xbf, 
  0xb3, 0x4a, 0xf1, 0x77, 0x79, 0x20, 0xfc, 0xdd, 
  0x0a, 0x4a, 0x34, 0x36, 0x2b, 0x25, 0x10, 0x78, 
  0x59, 0x29, 0x02, 0xbb, 0x07, 0x42, 0xe0, 0x8f, 
  0xbc, 0xd9, 0x2c, 0x81, 0xbf, 0x8b, 0xaa, 0xf0, 
  0x97, 0xcd, 0x57, 0x0a, 0x7b, 0x03, 0x18, 0x61, 
  0x7f, 0xed, 0x6d, 0x9d, 0x72, 0x6f, 0x96, 0x9f, 
  0x46, 0x54, 0xab, 0x6e, 0x2d, 0x5e, 0x70, 0x24, 
  0xaa, 0x1e, 0xed, 0x00, 0xc9, 0xdb, 0xd7, 0x23, 
  0x57, 0x35, 0xe8, 0xbb, 0x43, 0x12, 0x1f, 0x79, 
  0xaa, 0x41, 0x9b, 0x16, 0xe7, 0x3b, 0xe3, 0x23, 
  0x57, 0x35, 0xd8, 0xd2, 0xc7, 0x79, 0x17, 0x7a, 
  0xe4, 0xaa, 0x06, 0xed, 0x73, 0x71, 0xba, 0x72, 
  0xe4, 0xaa, 0x06, 0x3b, 0x75, 0x5e, 0x74, 0x97, 
  0x47, 0xb2, 0x6a, 0xd0, 0x16, 0x87, 0xb8, 0x03, 
  0x24, 0x41, 0xb5, 0x8a, 0x60, 0xba, 0x23, 0x6e, 
  0xca, 0xd7, 0xa3, 0x28, 0x60, 0x84, 0x25, 0x73, 
  0x13, 0x0d, 0x6a, 0xee, 0xbe, 0x18, 0xa0, 0xf4, 
  0x18, 0x3d, 0x32, 0x52, 0xc2, 0x13, 0xfc, 0xc5, 
  0x34, 0x8e, 0xa7, 0x66, 0x63, 0x19, 0xe7, 0x0b, 
  0x09, 0x81, 0x14, 0x09, 0xda, 0x30, 0x05, 0xbf, 
  0xa4, 0x78, 0x35, 0x8b, 0x2b, 0x1b, 0xd3, 0x98, 
  0x7f, 0xd1, 0xbf, 0xe9, 0xbd, 0xbb, 0xdb, 0x45, 
  0xd2, 0x7e, 0xbe, 0x48, 0x66, 0xb6, 0x2f, 0xc9, 
  0xe4, 0x5a, 0x8c, 0x72, 0x91, 0x68, 0x21, 0xb8, 
  0x4b, 0x64, 0x29, 0x22, 0x59, 0x7d, 0x86, 0xbc, 
  0x4e, 0xc9, 0xd7, 0x89, 0xd6, 0x58, 0x69, 0x26, 
  0x56, 0x4a, 0x08, 0x45, 0xfa, 0x77, 0x94, 0xbc, 
  0x05, 0x29, 0x81, 0x02, 0x4b, 0x41, 0x39, 0x1e, 
  0xf8, 0x24, 0xe1, 0x7a, 0xeb, 0x80, 0x2c, 0x80, 
  0x9a, 0xc5, 0x8e, 0x86, 0x91, 0x5e, 0xf9, 0x41, 
  0xe7, 0xdb, 0xc4, 0x76, 0x43, 0x01, 0x47, 0xff, 
  0xc0, 0xc3, 0x1b, 0x89, 0x4b, 0xde, 0x2b, 0xf5, 
  0x7a, 0x7e, 0x52, 0x85, 0x6a, 0x27, 0x7c, 0x6a, 
  0x2d, 0x6a, 0x77, 0x08, 0x28, 0xd0, 0x01, 0xa6, 
  0x0a, 0x07, 0x89, 0x8d, 0x77, 0xc2, 0x1c, 0x16, 
  0xf9, 0xc2, 0xc9, 0x03, 0xe1, 0xcc, 0x10, 0xea, 
  0x9e, 0xc4, 0x31, 0x64, 0xdf, 0x50, 0x44, 0xa6, 
  0xdc, 0x75, 0xb1, 0xfc, 0x14, 0x66, 0x85, 0x60, 
  0x12, 0x4a, 0x96, 0xdd, 0x96, 0x73, 0xc6, 0x1e, 
  0xb3, 0x11, 0x70, 0x47, 0xb1, 0xdf, 0x38, 0x6b, 
  0x6b, 0xd3, 0x1a, 0xc6, 0xa8, 0x88, 0x9c, 0xfc, 
  0x2b, 0x10, 0x67, 0x4a, 0x5b, 0xb9, 0x97, 0x5b, 
  0x06, 0x58, 0xc5, 0xc4, 0x2b, 0x2b, 0xfa, 0x52, 
  0x06, 0xac, 0xcb, 0xde, 0x69, 0x3d, 0xda, 0xe6, 
  0x8b, 0xcf, 0x38, 0xdf, 0x1a, 0x9b, 0x58, 0x28, 
  0x96, 0x2f, 0x8f, 0xcd, 0xfd, 0xaa, 0xf7, 0x9d, 
  0x08, 0x02, 0x0e, 0xf3, 0xca, 0xf7, 0x49, 0x2a, 
  0x24, 0xd3, 0x15, 0xd1, 0xfe, 0x23, 0x99, 0xea, 
  0x89, 0xb1, 0x2b, 0x65, 0x3a, 0xb5, 0xda, 0x99, 
  0x61, 0xf6, 0x14, 0x6a, 0xbb, 0xb2, 0x42, 0x85, 
  0x10, 0x8d, 0x08, 0xad, 0xa9, 0xa7, 0x95, 0x54, 
  0xe1, 0x9e, 0x7b, 0x88, 0x79, 0xfd, 0xa1, 0xc3, 
  0x11, 0x3e, 0xad, 0x67, 0xe9, 0xfd, 0x90, 0x1b, 
  0x6f, 0x68, 0xb1, 0x58, 0x7e, 0xf7, 0xa9, 0xbd, 
  0x45, 0x28, 0x9e, 0x5c, 0x7c, 0x67, 0x6b, 0xad, 
  0x2b, 0x46, 0xad, 0x0a, 0x2e, 0xb4, 0x8b, 0x39, 
  0xd7, 0xa9, 0x6c, 0xce, 0x85, 0xe0, 0x31, 0x62, 
  0x65, 0x1f, 0x62, 0x5c, 0xd4, 0xb2, 0xa5, 0x34, 
  0x91, 0xfa, 0x39, 0xab, 0x44, 0x3d, 0x16, 0x85, 
  0xc5, 0x93, 0xae, 0xb7, 0xa5, 0xf0, 0xb3, 0xfe, 
  0x84, 0xaa, 0xc8, 0xaa, 0x2a, 0x44, 0x74, 0xd7, 
  0xfd, 0xef, 0xff, 0x2e, 0xc4, 0xed, 0x5d, 0xed, 
  0x7c, 0xf8, 0x23, 0xe9, 0xc9, 0x1f, 0x25, 0xca, 
  0x7b, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x28, 0x75, 0x75, 0x61, 0x79, 0x29, 0x6f, 
  0x72, 0x67, 0x2f, 0x00, 0x05, 0x00, 0x00, 0x00, 
  0x2f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
  0x75, 0x69, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x04, 0x00, 0x00, 0x00, 0x70, 0x72, 0x65, 0x66, 
  0x65, 0x72, 0x65, 0x6e, 0x63, 0x65, 0x73, 0x2e, 
  0x75, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x16, 0x39, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
  0x78, 0xda, 0xed, 0x5b, 0xdd, 0x72, 0xa3, 0x36, 
  0x14, 0xbe, 0xdf, 0xa7, 0xa0, 0xdc, 0x76, 0x58, 
  0x1b, 0x3b, 0xc9, 0xa6, 0x1d, 0x87, 0x9d, 0x6e, 
  0x66, 0xb3, 0xd3, 0x99, 0x9d, 0x4e, 0xa7, 0xc9, 
  0x5e, 0x33, 0x32, 0x1c, 0x63, 0xad, 0x85, 0x44, 
  0x25, 0x11, 0xc7, 0x77, 0xfb, 0x2c, 0x7d, 0xb4, 
  0x3e, 0x49, 0x8f, 0x00, 0x3b, 0xd8, 0x0e, 0xe0, 
  0xfc, 0x60, 0x13, 0x37, 0x57, 0xb6, 0xe0, 0x48, 
  0x3a, 0x3f, 0x9f, 0xce, 0x1f, 0x30, 0xfa, 0x78, 
  0x17, 0x33, 0xeb, 0x16, 0xa4, 0xa2, 0x82, 0x5f, 
  0xd8, 0xee, 0xfb, 0xbe, 0x6d, 0x01, 0x0f, 0x44, 
  0x48, 0x79, 0x74, 0x61, 0x7f, 0xbb, 0xb9, 0x72, 
  0xce, 0xed, 0x8f, 0xde, 0xbb, 0x11, 0xe5, 0x1a, 
  0xe4, 0x84, 0x04, 0xe0, 0x8d, 0x7e, 0x72, 0x1c, 
  0x6b, 0x35, 0x74, 0x24, 0xfc, 0x9d, 0x52, 0x09, 
  0xca, 0x8a, 0xf4, 0xec, 0x67, 0x6b, 0xf8, 0xfe, 
  0xcc, 0x72, 0x1c, 0x6f, 0x24, 0xc6, 0xdf, 0x21, 
  0xd0, 0x56, 0xc0, 0x88, 0x52, 0x17, 0xf6, 0x17, 
  0x3d, 0xfb, 0x2d, 0xfc, 0x9e, 0x2a, 0x1d, 0x03, 
  0xd7, 0xb6, 0x45, 0xc3, 0x0b, 0x3b, 0x5b, 0xe0, 
  0x96, 0x30, 0xdb, 0x1b, 0x25, 0x52, 0x24, 0x20, 
  0xf5, 0xc2, 0xe2, 0x24, 0x86, 0x0b, 0x9b, 0x89, 
  0x39, 0x48, 0xdb, 0xeb, 0xbf, 0x3f, 0x1d, 0xf5, 
  0x96, 0xb7, 0xb6, 0x88, 0xd2, 0x24, 0x31, 0x44, 
  0xc3, 0x7e, 0x0d, 0x8d, 0xd2, 0x90, 0xf8, 0x94, 
  0x07, 0x12, 0xb2, 0x6d, 0x37, 0x57, 0x54, 0x34, 
  0xe2, 0x84, 0x15, 0xb4, 0xc8, 0x49, 0x0a, 0x4e, 
  0x30, 0x25, 0x3c, 0x82, 0xd0, 0xb6, 0xf0, 0x37, 
  0x64, 0x20, 0x2f, 0xec, 0x28, 0xa5, 0x7e, 0x22, 
  0x61, 0xa2, 0xfc, 0x25, 0xc3, 0xfe, 0x8a, 0x48, 
  0xcd, 0x09, 0x72, 0x81, 0xb2, 0x70, 0x61, 0xf7, 
  0xbc, 0x51, 0x2f, 0x97, 0xf9, 0x01, 0xd9, 0xbf, 
  0x52, 0xa5, 0xaf, 0xb5, 0x90, 0x90, 0x8b, 0xce, 
  0x70, 0xa8, 0xcc, 0xd0, 0x45, 0xe1, 0x03, 0xc1, 
  0xd2, 0x98, 0xab, 0x5c, 0xab, 0xf9, 0xc0, 0x31, 
  0x2c, 0x59, 0x9a, 0x6a, 0x06, 0x99, 0x2e, 0xf3, 
  0xab, 0x96, 0x5e, 0x24, 0xc8, 0x68, 0x84, 0xdb, 
  0x4b, 0x22, 0x25, 0x59, 0x98, 0x3d, 0x37, 0x27, 
  0xd1, 0xb0, 0x69, 0x46, 0x6f, 0xb5, 0x61, 0x48, 
  0x34, 0xf1, 0x46, 0x52, 0xcc, 0x33, 0xfa, 0x8c, 
  0x33, 0xb4, 0xbd, 0x96, 0x84, 0x2b, 0x86, 0xb7, 
  0xc6, 0x0c, 0xe7, 0x2e, 0x40, 0xd9, 0xde, 0xef, 
  0x81, 0xe0, 0x96, 0xe0, 0x6c, 0x91, 0x4d, 0xbe, 
  0xa7, 0x46, 0xf6, 0x29, 0xde, 0x72, 0x4a, 0xb7, 
  0x7a, 0xd9, 0x72, 0x3b, 0xac, 0xf9, 0xc5, 0x70, 
  0x6b, 0x58, 0xde, 0x5e, 0x53, 0x4d, 0x85, 0xd4, 
  0x8e, 0x86, 0x3b, 0xfd, 0xe4, 0x45, 0x2d, 0x34, 
  0x9f, 0x15, 0x99, 0x3f, 0xb1, 0x08, 0x51, 0x2b, 
  0x13, 0x6b, 0xc6, 0xc5, 0x9c, 0x6f, 0xef, 0x35, 
  0x49, 0x19, 0xdb, 0xde, 0xaa, 0x97, 0xab, 0xa6, 
  0xda, 0xa0, 0x9b, 0x60, 0x4e, 0x90, 0xe3, 0x4a, 
  0x20, 0xbb, 0xcd, 0x30, 0x3e, 0x3b, 0x3d, 0x1d, 
  0x9e, 0x3d, 0x02, 0xc9, 0x75, 0x4b, 0x26, 0x24, 
  0x82, 0x35, 0xda, 0xfe, 0x33, 0x40, 0x6f, 0x04, 
  0x7b, 0x02, 0xe0, 0xbf, 0x48, 0x1a, 0xe6, 0x9a, 
  0x91, 0x42, 0x3c, 0xa0, 0x99, 0x5b, 0xaa, 0x28, 
  0x9a, 0xcd, 0xf6, 0x6e, 0x64, 0x0a, 0x35, 0xb2, 
  0x04, 0x84, 0xfb, 0x13, 0x11, 0xa4, 0x68, 0xdb, 
  0x2b, 0xc2, 0x54, 0x1d, 0x29, 0xca, 0x82, 0xa2, 
  0x21, 0x7a, 0x34, 0x91, 0xba, 0x86, 0x6e, 0x2c, 
  0x64, 0x08, 0xd2, 0x9f, 0xd3, 0x50, 0x4f, 0x6d, 
  0xef, 0xbc, 0x86, 0x12, 0xa1, 0xe0, 0xab, 0x84, 
  0x04, 0xe8, 0x08, 0x6d, 0x6f, 0x50, 0xc7, 0x65, 
  0x76, 0xa2, 0xee, 0x69, 0xd7, 0x16, 0x0d, 0xa6, 
  0x94, 0x85, 0x0f, 0xa8, 0xe8, 0x93, 0xb8, 0xcb, 
  0x35, 0x34, 0x16, 0x77, 0xee, 0x5e, 0x34, 0x34, 
  0x85, 0xbb, 0x04, 0x2d, 0xdc, 0xb8, 0xe4, 0xe3, 
  0xe4, 0xf8, 0x4a, 0xc6, 0xc0, 0x0a, 0xbf, 0x66, 
  0xfe, 0xee, 0x47, 0x16, 0x96, 0xef, 0xfa, 0x90, 
  0x0f, 0x00, 0x0e, 0x92, 0xb0, 0xf2, 0x5c, 0xa2, 
  0xb5, 0xa4, 0xe3, 0x54, 0x83, 0x2a, 0xfd, 0x2f, 
  0x16, 0x9a, 0x03, 0x8d, 0xa6, 0x78, 0x8a, 0xb3, 
  0xa3, 0x60, 0x8c, 0xc1, 0xc2, 0x0c, 0xda, 0xe5, 
  0x39, 0x2b, 0x9c, 0xa3, 0x62, 0x66, 0xa8, 0x99, 
  0x2d, 0x6e, 0x96, 0x8a, 0x6d, 0xe2, 0x7a, 0x42, 
  0x19, 0x6b, 0xd4, 0x42, 0x22, 0x14, 0xd5, 0x18, 
  0x8b, 0x31, 0x5e, 0x95, 0xa9, 0x7a, 0xab, 0xcd, 
  0x7b, 0x85, 0x29, 0xaa, 0x2c, 0x72, 0x0d, 0x09, 
  0xfa, 0x7b, 0x8c, 0x2f, 0xb9, 0x55, 0xd4, 0x72, 
  0xd8, 0x92, 0x65, 0x76, 0xd6, 0x4e, 0xc3, 0x1e, 
  0x8f, 0x54, 0x8e, 0x5b, 0xaf, 0x9c, 0x46, 0xae, 
  0x18, 0x4c, 0xb4, 0x8f, 0x56, 0x26, 0xc1, 0x74, 
  0x43, 0xd3, 0x1b, 0x84, 0x5a, 0x24, 0x3b, 0xd1, 
  0x15, 0x1e, 0x65, 0x50, 0x7b, 0x02, 0x33, 0xb0, 
  0x35, 0xf1, 0x5e, 0x69, 0xd8, 0x84, 0xf2, 0x4f, 
  0xa9, 0xd6, 0x28, 0x7e, 0x6e, 0x59, 0x1c, 0x8f, 
  0xb3, 0xf1, 0xcb, 0x98, 0xb6, 0x81, 0x72, 0x57, 
  0xff, 0x41, 0x79, 0xb1, 0xb7, 0x09, 0x1c, 0x18, 
  0xdc, 0xfe, 0xfd, 0xf1, 0x4f, 0x0d, 0x35, 0xb9, 
  0x0f, 0xa5, 0xde, 0x32, 0xc5, 0xaa, 0x63, 0x97, 
  0xd1, 0x78, 0xec, 0x23, 0x9e, 0xa1, 0x29, 0x43, 
  0x0c, 0x69, 0x44, 0xb5, 0xda, 0x54, 0xf6, 0xa3, 
  0x60, 0xe1, 0xee, 0x08, 0x0b, 0xb7, 0x19, 0x16, 
  0x6e, 0x7b, 0xb0, 0xd8, 0xf4, 0xc0, 0x83, 0xbd, 
  0x78, 0xe0, 0x98, 0xc8, 0x88, 0x72, 0xdf, 0xe8, 
  0xab, 0x3e, 0x8c, 0xde, 0x15, 0x81, 0xb9, 0xff, 
  0x24, 0x77, 0xfe, 0x2d, 0xc1, 0x5c, 0x0c, 0xac, 
  0x55, 0xf6, 0xfd, 0x6b, 0x5d, 0x32, 0xa5, 0xc0, 
  0x4f, 0x39, 0x46, 0x77, 0x46, 0x79, 0xb3, 0xac, 
  0x31, 0x87, 0x58, 0x70, 0x1a, 0x98, 0x54, 0x20, 
  0x02, 0x14, 0xa2, 0x74, 0x9c, 0x9e, 0x0e, 0x99, 
  0xfe, 0xab, 0x84, 0xcc, 0xf0, 0x98, 0x20, 0xe3, 
  0x67, 0x65, 0x40, 0x9e, 0x94, 0xb5, 0x08, 0x17, 
  0x53, 0x5e, 0x38, 0xf9, 0x2e, 0xed, 0xc3, 0x65, 
  0xd0, 0x29, 0xb8, 0x9c, 0x1c, 0x15, 0x5c, 0xfe, 
  0xc4, 0x1a, 0x67, 0x2f, 0x6e, 0x65, 0xd0, 0x3e, 
  0x4e, 0x86, 0x9d, 0xc2, 0xc9, 0xe9, 0x51, 0xe1, 
  0xe4, 0x52, 0x70, 0x9e, 0xc9, 0x2b, 0xe2, 0x18, 
  0xf3, 0xa0, 0x16, 0x21, 0x13, 0xe4, 0x3b, 0x39, 
  0xc5, 0x4e, 0xed, 0xc3, 0xe6, 0xe4, 0xa0, 0xb0, 
  0xa9, 0xce, 0x6b, 0x07, 0xc7, 0x91, 0xd7, 0x9a, 
  0x2e, 0x4a, 0xfb, 0x59, 0xe8, 0x61, 0xcf, 0xfe, 
  0x67, 0xae, 0xe5, 0x22, 0xb7, 0xdf, 0x06, 0x7c, 
  0x5f, 0x91, 0x0d, 0xd7, 0xba, 0x63, 0x75, 0x7d, 
  0xb1, 0x42, 0xc4, 0xe6, 0xd6, 0xd8, 0x8b, 0x5a, 
  0xf8, 0xb0, 0xc7, 0xb4, 0xdc, 0xb1, 0xda, 0x6b, 
  0x8d, 0x81, 0x2a, 0xb0, 0xbd, 0xb3, 0x83, 0x77, 
  0xb6, 0xce, 0x0e, 0xdc, 0xd9, 0xfa, 0x43, 0x68, 
  0x3a, 0xa1, 0x01, 0x31, 0xfd, 0x0f, 0xf5, 0xd6, 
  0xdf, 0xda, 0xea, 0x6f, 0x0d, 0xde, 0xfa, 0x5b, 
  0xcf, 0xcd, 0x03, 0x4e, 0x0f, 0xda, 0xdf, 0xba, 
  0x9c, 0x42, 0x30, 0x2b, 0x27, 0x02, 0xbc, 0x84, 
  0x78, 0x07, 0x38, 0xc9, 0x4c, 0xb9, 0xfb, 0x81, 
  0xf9, 0xec, 0x67, 0x53, 0xea, 0x1e, 0x1a, 0xbc, 
  0x78, 0x20, 0x92, 0x10, 0x00, 0xbd, 0x05, 0xe5, 
  0x87, 0x30, 0x21, 0x29, 0xd3, 0x2f, 0x98, 0xbe, 
  0xee, 0xea, 0xe5, 0x1e, 0x97, 0x7c, 0xee, 0x92, 
  0x14, 0x87, 0x92, 0xcc, 0x7d, 0xca, 0x43, 0x63, 
  0x0a, 0x3c, 0x76, 0x5b, 0x4b, 0xae, 0x85, 0x4d, 
  0x2d, 0xa2, 0x88, 0x55, 0x84, 0xcd, 0xdc, 0x86, 
  0xfe, 0x8a, 0xe4, 0x59, 0x51, 0x73, 0x57, 0x50, 
  0x9f, 0x1d, 0x14, 0xd4, 0x9b, 0x51, 0xe4, 0xc3, 
  0x51, 0xd5, 0x44, 0xd7, 0x22, 0x6d, 0xb5, 0x12, 
  0x5a, 0xf3, 0x00, 0xca, 0x6c, 0xd6, 0x7e, 0x31, 
  0xf4, 0xa1, 0x33, 0x59, 0xd6, 0xb0, 0x9b, 0xcf, 
  0x05, 0x4f, 0x76, 0xc9, 0x9e, 0xae, 0x28, 0x83, 
  0xcb, 0xa9, 0x10, 0x0a, 0x64, 0xa5, 0x4b, 0xcf, 
  0x0c, 0xda, 0x29, 0x19, 0x85, 0xa4, 0x58, 0xb1, 
  0x91, 0x3c, 0x04, 0xdf, 0xe2, 0x0d, 0x64, 0x95, 
  0x55, 0xba, 0x3a, 0x0c, 0xe9, 0xe0, 0x28, 0xc4, 
  0xe9, 0x43, 0xbe, 0x2e, 0x13, 0xee, 0xe9, 0x05, 
  0x42, 0xb7, 0x72, 0xae, 0xb2, 0x09, 0x2b, 0x1f, 
  0x39, 0xe5, 0x7e, 0xc2, 0x8b, 0xf4, 0xcc, 0x89, 
  0x21, 0xa4, 0xc4, 0x49, 0x18, 0x59, 0x1c, 0x38, 
  0xfa, 0xee, 0x10, 0x2b, 0x95, 0x16, 0xc1, 0xac, 
  0x21, 0xa8, 0x05, 0x8c, 0x06, 0xb3, 0x8a, 0xa0, 
  0x96, 0x1b, 0x1a, 0x33, 0x69, 0x2c, 0x07, 0x97, 
  0x64, 0x1d, 0xb1, 0xf6, 0x8b, 0x26, 0x91, 0xee, 
  0xab, 0xf3, 0x9f, 0x27, 0xff, 0xc3, 0x2a, 0xf5, 
  0xfc, 0xc0, 0x55, 0xea, 0x25, 0x16, 0xa7, 0x62, 
  0x3d, 0xe9, 0x7e, 0xab, 0x4f, 0xb3, 0xe1, 0xf0, 
  0xad, 0x3e, 0x7d, 0x6e, 0x6a, 0x76, 0xde, 0xad, 
  0xfa, 0x54, 0x2d, 0x94, 0x86, 0xd8, 0x99, 0x08, 
  0xae, 0x1f, 0x53, 0x97, 0x7e, 0x53, 0x60, 0x29, 
  0x3f, 0x9b, 0x6b, 0x61, 0xba, 0x2b, 0xcc, 0x81, 
  0x07, 0xcb, 0xac, 0xf2, 0x56, 0xaa, 0x76, 0xa6, 
  0x54, 0x45, 0xdb, 0x1a, 0x8b, 0xec, 0xb9, 0x56, 
  0xfd, 0xa5, 0x2b, 0xb5, 0xaa, 0x91, 0xdd, 0x29, 
  0x12, 0xba, 0x23, 0xaa, 0x57, 0xaf, 0x50, 0xac, 
  0x16, 0xcb, 0xd5, 0xcd, 0x33, 0xdc, 0xd2, 0xbb, 
  0x23, 0xfd, 0x4e, 0x3d, 0xe5, 0x75, 0xfb, 0x47, 
  0x05, 0x91, 0xaf, 0x22, 0xb2, 0x48, 0x18, 0x4a, 
  0x50, 0xea, 0xf5, 0x23, 0xe5, 0xb0, 0xaf, 0x19, 
  0x99, 0xd3, 0x56, 0x8e, 0x97, 0x0f, 0x07, 0xca, 
  0x0e, 0x94, 0x81, 0xbb, 0xc6, 0xab, 0x5c, 0x80, 
  0x6b, 0x04, 0x8d, 0xe5, 0xd6, 0xf9, 0x61, 0x0c, 
  0x21, 0xb7, 0x14, 0xe6, 0xbe, 0xf9, 0xb2, 0xc0, 
  0x04, 0x8b, 0xf5, 0xae, 0x81, 0x71, 0xad, 0x55, 
  0x5d, 0x83, 0x2c, 0xe6, 0xec, 0xf7, 0xa9, 0xe2, 
  0x81, 0xdd, 0x49, 0xe9, 0xc1, 0x31, 0x13, 0x91, 
  0x53, 0x1c, 0xbd, 0x63, 0x7c, 0x68, 0x8c, 0xe2, 
  0x15, 0xd2, 0xed, 0xdb, 0xc2, 0x87, 0x75, 0x03, 
  0x97, 0x22, 0x1e, 0x8b, 0x55, 0x5d, 0x5e, 0x7a, 
  0x6f, 0xae, 0x53, 0xad, 0x3f, 0xf3, 0xa5, 0x10, 
  0x26, 0x3b, 0xf7, 0x1f, 0x67, 0xd5, 0xbd, 0xdc, 
  0x11, 0x68, 0xf4, 0x2f, 0xf5, 0x7e, 0x98, 0x86, 
  0xfe, 0x52, 0x4a, 0xf7, 0x49, 0x70, 0x31, 0x8a, 
  0x2a, 0x3e, 0x2d, 0xa9, 0x82, 0x4b, 0xa5, 0xc6, 
  0x81, 0xb1, 0xbf, 0xc0, 0x44, 0x27, 0x90, 0x37, 
  0xc6, 0x07, 0xe5, 0xef, 0x65, 0xe0, 0x55, 0x59, 
  0x5c, 0x35, 0x9e, 0xc9, 0x35, 0x4b, 0xd4, 0x15, 
  0xe7, 0x99, 0xfb, 0x32, 0x42, 0xae, 0x6e, 0x6c, 
  0x56, 0xe6, 0xad, 0x36, 0x92, 0xf6, 0xf3, 0xd2, 
  0xe3, 0x8a, 0xf7, 0xde, 0xfd, 0x27, 0x8e, 0xef, 
  0xfe, 0x03, 0xde, 0x93, 0xe3, 0xba, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x28, 0x75, 0x75, 0x61, 0x79, 0x29, 0x67, 
  0x73, 0x77, 0x61, 0x74, 0x63, 0x68, 0x65, 0x72, 
  0x2f, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00
} };

static GStaticResource static_resource = { gswatcher_resource_data.data, sizeof (gswatcher_resource_data.data), NULL, NULL, NULL };
extern GResource *gswatcher_get_resource (void);
GResource *gswatcher_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a sane way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
/* Visual studio 2008 and later has _Pragma */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _wrapper(void) { _func(); return 0; } \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _wrapper;

#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _constructor(void) { atexit (_func); return 0; } \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;

#elif defined (_MSC_VER)

#define G_HAS_CONSTRUCTORS 1

/* Pre Visual studio 2008 must use #pragma section */
#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _wrapper(void) { _func(); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (*p)(void) = _func ## _wrapper;

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _constructor(void) { atexit (_func); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif


#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(resource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(resource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(resource_destructor)
#endif
G_DEFINE_DESTRUCTOR(resource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void resource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void resource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}
