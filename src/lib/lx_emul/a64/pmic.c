/*
 * \brief  PMIC stub driver to avoid the infinite deferral dwmac-sun8i driver
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/sunxi-rsb.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#define RSB_CTRL_NAME "sunxi-rsb"


int sunxi_rsb_driver_register(struct sunxi_rsb_driver *rdrv)
{
	return 0;
}


static int sunxi_rsb_probe(struct platform_device *pdev)
{
	return 0;
}


static const struct of_device_id sunxi_rsb_of_match_table[] = {
	{ .compatible = "allwinner,sun8i-a23-rsb" },
	{ }
};


MODULE_DEVICE_TABLE(of, sunxi_rsb_of_match_table);


static struct platform_driver sunxi_rsb_driver = {
	.probe    = sunxi_rsb_probe,
	.driver   = {
		.name           = RSB_CTRL_NAME,
		.of_match_table = sunxi_rsb_of_match_table,
	},
};


static int __init sunxi_rsb_init(void)
{
	return platform_driver_register(&sunxi_rsb_driver);
}


module_init(sunxi_rsb_init);

