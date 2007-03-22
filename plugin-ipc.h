/*
 * copyright (c) 2006 by embrix.
 * 
 * all rights reserved.
 * 
 * developed by embrix for corrigent systems ltd.
 */

#ifndef __PEDRO_IPC_H__
#define __PEDRO_IPC_H__

int ipc_init(void);
int ipc_cmd_handler(int argc, char* argv[]);
int ipc_cleanup(void);

#endif /* __PEDRO_IPC_H__ */
