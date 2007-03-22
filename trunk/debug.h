#ifndef __DEBUG_H__
#define __DEBUG_H__

//
// defines
// 

#ifdef PEDRO_DEBUG
#define trace(fmt, args...) fprintf(stderr, "=> " fmt, ##args)
#else
#define trace(fmt, args...)
#endif


#endif /* __DEBUG_H__ */
