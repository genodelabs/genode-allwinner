/**
 * \brief  Lx_emul support to handle reset pins
 * \author Stefan Kalkowski
 * \date   2021-05-17
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifndef _LX_EMUL__RESET_H_
#define _LX_EMUL__RESET_H_

#ifdef __cplusplus
extern "C" {
#endif

void lx_emul_reset_deassert(const char * name);

#ifdef __cplusplus
}
#endif

#endif /* _LX_EMUL__RESET_H_ */

