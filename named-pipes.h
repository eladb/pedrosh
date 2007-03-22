/**
 * named-pipes.h
 * implements named full-duplex pipes.
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 10, 2006
 */

#ifndef __NAMED_PIPES_H__
#define __NAMED_PIPES_H__

int named_pipes(const char* ic, const char* og, int timeout_ms, int timeout_once);

#endif /* __NAMED_PIPES_H__ */
