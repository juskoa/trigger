#ifndef _VMEGLUE_H 
#define _VMEGLUE_H

// Definitions
#define MODE_A24 1
#define MODE_A32 2

// Prototypes
u_int VME_start(void);
u_int VME_stop(void);

u_int VME_map(u_int am, u_int base, u_int size, u_int *handle);
u_int VME_unmap(u_int handle);

u_int VME_d32r(u_int handle, u_int offset, u_int *data);
u_int VME_d32w(u_int handle, u_int offset, u_int data);

u_int VME_d16r(u_int handle, u_int offset, u_short *data);
u_int VME_d16w(u_int handle, u_int offset, u_short data);

#endif
