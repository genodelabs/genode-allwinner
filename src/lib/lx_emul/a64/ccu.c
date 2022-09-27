/*
 * \brief  CCU stub driver to avoid the infinite deferral of i2c camera devices
 * \author Josef Soentgen
 * \date   2022-09-27
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

static int sun50i_a64_ccu_probe(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id sun50i_a64_ccu_ids[] = {
    { .compatible = "allwinner,sun50i-a64-ccu" },
    { }
};

static struct platform_driver sun50i_a64_ccu_driver = {
    .probe  = sun50i_a64_ccu_probe,
    .driver = {
        .name   = "sun50i-a64-ccu",
        .of_match_table = sun50i_a64_ccu_ids,
    },
};
builtin_platform_driver(sun50i_a64_ccu_driver);
