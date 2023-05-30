/*
 * \brief  Emulation of the Linux clock interfaces
 * \author Josef Soentgen
 * \date   2023-03-10
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <lx_emul/clock.h>

struct wifi_clock_mapping {
	char const    *client_con_id;
	char const    *clock_name;
	struct clk    *clk;
	struct clk_hw *hw;
};

enum { NUM_WIFI_CLOCK_MAPPINGS = 3 };

static struct wifi_clock_mapping wifi_clock_mappings[NUM_WIFI_CLOCK_MAPPINGS] = {
	{ "ahb", "bus-mmc1",  NULL, NULL },
	{ "mmc", "mmc1",  NULL, NULL },
	{ "ext_clock", "ext_clock",  NULL, NULL }
};


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
		for (i = 0; i < NUM_WIFI_CLOCK_MAPPINGS; i++) {

			struct wifi_clock_mapping *mapping = &wifi_clock_mappings[i];

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

	if (!res) {
		printk("%s:%d: no clock found for '%s'\n", __func__, __LINE__,
		        con_id);
		lx_emul_trace_and_stop(__func__);
	}

	return res;
}


static struct clk_hw *lookup_internal_clk_hw(struct clk *clk)
{
	unsigned i;
	for (i = 0; i < NUM_WIFI_CLOCK_MAPPINGS; i++) {
		struct wifi_clock_mapping *mapping = &wifi_clock_mappings[i];
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


long clk_round_rate(struct clk * clk,unsigned long rate)
{
	lx_emul_trace(__func__);
	return rate;
}
