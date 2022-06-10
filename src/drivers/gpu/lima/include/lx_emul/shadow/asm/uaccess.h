/**
 * \brief  Shadow copy of linux/uaccess.h
 * \author Josef Soentgen
 * \date   2021-04-19
 */

#pragma once

extern unsigned long emul_user_copy(void *, void const*, unsigned long);

#include_next <asm/uaccess.h>

#undef access_ok
#define access_ok(addr, size) 1

#undef raw_copy_from_user
#define raw_copy_from_user(to, from, n) emul_user_copy((to), (from), (n))

#undef raw_copy_to_user
#define raw_copy_to_user(to, from, n) emul_user_copy((to), (from), (n))
