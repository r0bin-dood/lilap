#ifndef LCU_H
#define LCU_H

typedef int bool;
#define false (0)
#define true  (!false)

#define UNUSED(x) ((void)(x))

#define IGNORE_RET(x) ((void)(x))

typedef void (*lcu_cleanup_callback)(void *);

#endif // LCU_H