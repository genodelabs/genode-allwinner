/*
 * \brief  Emulation of the Linux reset interfaces
 * \author Norman Feske
 * \date   2021-11-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/device.h>
#include <linux/of.h>
#include <linux/reset.h>
#include <linux/reset-controller.h>
#include <lx_emul/reset.h>

static struct reset_controller_dev *reset_controller_de2_clk;


struct de2_clk_reset {
	char const *name;
	unsigned line;
};


static struct de2_clk_reset de2_clk_resets[] = {
	{ "allwinner,sun50i-a64-de2-mixer-0", 0 },
	{ "allwinner,sun50i-a64-de2-mixer-1", 1 },
	{ NULL, 0 } };


struct reset_control * __devm_reset_control_get(struct device * dev,
                                                const char    * id,
                                                int             index,
                                                bool            shared,
                                                bool            optional,
                                                bool            acquired)
{
	struct de2_clk_reset *reset;

	for (reset = &de2_clk_resets[0]; reset->name; reset++)
		if (of_device_is_compatible(dev->of_node, reset->name))
			return (struct reset_control *)reset->name;

	return NULL;
}


int reset_control_deassert(struct reset_control * rstc)
{
	char const * const name = (char const *)rstc;
	struct de2_clk_reset *reset;

	if (!rstc)
		return 0;

	if (!reset_controller_de2_clk) {
		printk("Error: reset_control_deassert de2_clk reset controller not yet known\n");
		return -EPROBE_DEFER;
	}

	/*
	 * Apply deassert request that refers to the de2_clk reset controller
	 */
	for (reset = &de2_clk_resets[0]; reset->name; reset++)
		if (strcmp(name, reset->name) == 0)
			reset_controller_de2_clk->ops->deassert(reset_controller_de2_clk, reset->line);

	return 0;
}


/*
 * Called by drivers/clk/sunxi-ng/ccu-sun8i-de2.c:345
 */

int reset_controller_register(struct reset_controller_dev * rcdev)
{
	int len = 0;
	struct property *property = of_find_property(rcdev->of_node, "compatible", &len);
	if (property) {

		if (strncmp((char *)property->value, "allwinner,sun50i-a64-de2-clk", len) == 0) {
			reset_controller_de2_clk = rcdev;
			return 0;
		}
	}

	/* unexpected */
	lx_emul_trace_and_stop(__func__);
	return 0;
}


#include <linux/reset.h>

int reset_control_reset(struct reset_control * rstc)
{
	lx_emul_trace(__func__);
	return 0;
}
