/**
 * \brief  Backend implementation for Linux
 * \author Josef Soentgen
 * \date   2021-03-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifndef _LX_EMUL_CC_H_
#define _LX_EMUL_CC_H_

#ifdef __cplusplus
extern "C" {
#endif

unsigned long emul_user_copy(void *, void const*, unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* _LX_EMUL_CC_H_ */
