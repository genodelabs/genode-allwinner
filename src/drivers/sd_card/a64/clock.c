/*
 * \brief  Emulation of the Linux clock interfaces
 * \author Norman Feske
 * \author Josef Soentgen
 * \date   2021-11-25
 */

/*
 * Copyright (C) 2021-2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <lx_emul/clock.h>

struct gpu_block_mapping {
	char const    *client_con_id;
	char const    *clock_name;
	struct clk    *clk;
	struct clk_hw *hw;
};

enum { NUM_GPU_CLOCK_MAPPINGS = 2 };

static struct gpu_block_mapping gpu_block_mappings[NUM_GPU_CLOCK_MAPPINGS] = {
	{ "ahb", "bus-mmc0",  NULL, NULL },
	{ "mmc", "mmc0",  NULL, NULL } };


struct clk *clk_get(struct device *dev, const char *con_id)
{
	struct clk *res = NULL;

	if (!dev || !dev->of_node) {
		printk("Error: clk_get unexpectedly called with no dev!\n");
		lx_emul_trace_and_stop(__func__);
		return NULL;
	}

	{
		unsigned i;
		for (i = 0; i < NUM_GPU_CLOCK_MAPPINGS; i++) {

			struct gpu_block_mapping *mapping = &gpu_block_mappings[i];

			if (strcmp(con_id, mapping->client_con_id) != 0)
				continue;

			if (!mapping->clk)
				mapping->clk = lx_emul_clock_get(dev->parent->of_node, mapping->clock_name);

			res = mapping->clk;
			break;
		}
	}

	if (!res)
		res = lx_emul_clock_get(dev->of_node, con_id);

	if (!res)
		lx_emul_trace_and_stop(__func__);

	return res;
}


static struct clk_hw *lookup_internal_clk_hw(struct clk *clk)
{
	unsigned i;
	for (i = 0; i < NUM_GPU_CLOCK_MAPPINGS; i++) {
		struct gpu_block_mapping *mapping = &gpu_block_mappings[i];
		if (mapping->clk == clk)
			return mapping->hw;
	}

	return NULL;
}


const char *__clk_get_name(const struct clk *clk)
{
	lx_emul_trace(__func__);
	return "unknown-clk";
}


unsigned long clk_get_rate(struct clk * clk)
{
	return lx_emul_clock_get_rate(clk);
}


int clk_set_rate(struct clk * clk, unsigned long rate)
{
	/*
	 * The SD/MMC driver tries to set the clock rate from
	 * 400000 to 25000000 and the platform driver does not
	 * export the proper rate anyway. So we silently ignore
	 * the request but report success.
	 */
	return 0;
}


int clk_enable(struct clk * clk)
{
	struct clk_hw *hw = lookup_internal_clk_hw(clk);
	if (hw)
		hw->init->ops->enable(hw);

	return 0;
}


void clk_put(struct clk *clk) { }


int clk_prepare(struct clk * clk) { return 0; }


void clk_disable(struct clk * clk) { }


void clk_unprepare(struct clk * clk) { }


void of_clk_init(const struct of_device_id *matches) { }


struct clk * devm_clk_get(struct device * dev,const char * id)
{
	return clk_get(dev, id);
}


struct clk * devm_clk_get_optional(struct device * dev,const char * id)
{
	return devm_clk_get(dev, id);
}


long clk_round_rate(struct clk * clk,unsigned long rate)
{
	lx_emul_trace(__func__);
	return rate;
}
