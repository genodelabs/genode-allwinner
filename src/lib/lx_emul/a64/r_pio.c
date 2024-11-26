/*
 * \brief  Stub variant of the Linux Kernel's A64 r-pinctrl driver
 * \author Josef Soentgen
 * \date   2024-11-28
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/of.h>
#include <linux/platform_device.h>


static int a64_pinctrl_probe(struct platform_device *pdev)
{
	return 0;
}


static const struct of_device_id a64_r_pinctrl_match[] = {
	{ .compatible = "allwinner,sun50i-a64-r-pinctrl", },
	{ /* sentinel */ }
};


static struct platform_driver sun50i_a64_r_pinctrl_driver = {
	.probe  = a64_pinctrl_probe,
	.driver = {
		.name           = "sun50i-a64-r-pinctrl",
		.of_match_table = a64_r_pinctrl_match,
	},
};


builtin_platform_driver(sun50i_a64_r_pinctrl_driver);
