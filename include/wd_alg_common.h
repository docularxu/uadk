// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright 2020-2021 Huawei Technologies Co.,Ltd. All rights reserved.
 * Copyright 2020-2021 Linaro ltd.
 */

#ifndef WD_ALG_COMMON_H
#define WD_ALG_COMMON_H

#include <pthread.h>
#include <stdbool.h>
#include "wd.h"
#include "wd_alg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BYTE_BITS			8
#define BYTE_BITS_SHIFT			3
#define GET_NEGATIVE(val)		(0 - (val))

#define BITS_TO_BYTES(bits)	(((bits) + 7) >> 3)
#define BYTES_TO_BITS(bytes)	((bytes) << 3)

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))
#define MAX_STR_LEN		256
#define CTX_TYPE_INVALID	9999
#define POLL_TIME		1000

enum alg_task_type {
	TASK_MIX = 0x0,
	TASK_HW,
	TASK_INSTR,
	TASK_MAX_TYPE,
};

enum wd_ctx_mode {
	CTX_MODE_SYNC = 0,
	CTX_MODE_ASYNC,
	CTX_MODE_MAX,
};

/**
 * struct wd_ctx - Define one ctx and related type.
 * @ctx:	The ctx itself.
 * @op_type:	Define the operation type of this specific ctx.
 *		e.g. 0: compression; 1: decompression.
 * @ctx_mode:   Define this ctx is used for synchronization of asynchronization
 *		1: synchronization; 0: asynchronization;
 */
struct wd_ctx {
	handle_t ctx;
	__u8 op_type;
	__u8 ctx_mode;
};

/**
 * struct wd_ctx_config - Define a ctx set and its related attributes, which
 *			  will be used in the scope of current process.
 * @ctx_num:	The ctx number in below ctx array.
 * @ctxs:	Point to a ctx array, length is above ctx_num.
 * @priv:	The attributes of ctx defined by user, which is used by user
 *		defined scheduler.
 */
struct wd_ctx_config {
	__u32 ctx_num;
	struct wd_ctx *ctxs;
	void *priv;
};

/**
 * struct wd_ctx_nums - Define the ctx sets numbers.
 * @sync_ctx_num: The ctx numbers which are used for sync mode for each
 * ctx sets.
 * @async_ctx_num: The ctx numbers which are used for async mode for each
 * ctx sets.
 */
struct wd_ctx_nums {
	__u32 sync_ctx_num;
	__u32 async_ctx_num;
};

/**
 * struct wd_ctx_params - Define the ctx sets params which are used for init
 * algorithms.
 * @op_type_num: Used for index of ctx_set_num, the order is the same as
 * wd_<alg>_op_type.
 * @ctx_set_num: Each operation type ctx sets numbers.
 * @bmp: Ctxs distribution. Means users want to run business process on these
 * numa or request ctx from devices located in these numa.
 */
struct wd_ctx_params {
	__u32 op_type_num;
	struct wd_ctx_nums *ctx_set_num;
	struct bitmask *bmp;
};

struct wd_ctx_internal {
	handle_t ctx;
	__u8 op_type;
	__u8 ctx_mode;
	__u16 sqn;
	pthread_spinlock_t lock;
};

struct wd_ctx_config_internal {
	__u32 ctx_num;
	int shmid;
	struct wd_ctx_internal *ctxs;
	void *priv;
	bool epoll_en;
	unsigned long *msg_cnt;
};

/**
 * struct wd_comp_sched - Define a scheduler.
 * @name:		Name of this scheduler.
 * @sched_policy:	Method for scheduler to perform scheduling
 * @sched_init: 	inited the scheduler input parameters.
 * @pick_next_ctx:	Pick the proper ctx which a request will be sent to.
 *			config points to the ctx config; sched_ctx points to
 *			scheduler context; req points to the request. Return
 *			the proper ctx pos in wd_ctx_config.
 *			(fix me: modify req to request?)
 * @poll_policy:	Define the polling policy. config points to the ctx
 *			config; sched_ctx points to scheduler context; Return
 *			number of polled request.
 */
struct wd_sched {
	const char *name;
	int sched_policy;
	handle_t (*sched_init)(handle_t h_sched_ctx, void *sched_param);
	__u32 (*pick_next_ctx)(handle_t h_sched_ctx,
				  void *sched_key,
				  const int sched_mode);
	int (*poll_policy)(handle_t h_sched_ctx, __u32 expect, __u32 *count);
	handle_t h_sched_ctx;
};

typedef int (*wd_alg_init)(struct wd_ctx_config *config, struct wd_sched *sched);
typedef int (*wd_alg_poll_ctx)(__u32 idx, __u32 expt, __u32 *count);

struct wd_datalist {
	void *data;
	__u32 len;
	struct wd_datalist *next;
};

#ifdef __cplusplus
}
#endif

#endif
