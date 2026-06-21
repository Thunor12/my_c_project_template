#ifndef NOB_MACROS_H
#define NOB_MACROS_H

#define HANDLE_COM_RES(RES, SYNC) \
    switch (RES)                  \
    {                             \
    case COMP_RES__FAILED:        \
        goto defer;               \
    case COMP_RES__OK:            \
        (SYNC) = 1;               \
        break;                    \
    default:                      \
        break;                    \
    }

#define MARK_BUILT(RES, FLAG)              \
    do                                     \
    {                                      \
        if ((RES) != COMP_RES__FAILED)     \
            (FLAG) = 1;                    \
    } while (0)

#endif
