/*
 * \brief  Emulation of the Linux clock interfaces
 * \author Josef Soentgen
 * \date   2022-09-26
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <lx_emul/clock.h>


static struct dummy_clock
{
	char const    *name;
	unsigned long  rate;
} _dummy_clks[] = {
	{ "bus",  200000000ul }, /* bus  - bus-csi  attached at ahb1 */
	{ "mod",  300000000ul }, /* mod  - csi-sclk attached at pll-periph0 */
	{ "ram", 1056000000ul }, /* ram  - dram-csi attached at pll-ddr1 */
	{ "xclk",  24000000ul }, /* xclk - csi-mclk attached at osc24M */
};

#define CLK_COUNT (sizeof(_dummy_clks)/sizeof(_dummy_clks[0]))


struct clk *clk_get(struct device *dev, const char *con_id)
{
	size_t i;

	if (!dev || !dev->of_node) {
		printk("Error: clk_get unexpectedly called with no dev!\n");
		lx_emul_trace_and_stop(__func__);
		return NULL;
	}

	for (i = 0; i < CLK_COUNT; i++)
		if (strcmp(con_id, _dummy_clks[i].name) == 0)
			return (struct clk*)&_dummy_clks[i];

	return NULL;
}


const char *__clk_get_name(const struct clk *clk)
{
	size_t i;
	for (i = 0; i < CLK_COUNT; i++)
		if (clk == (struct clk const*)&_dummy_clks[i])
			return _dummy_clks[i].name;

	return "unknown-clk";
}


unsigned long clk_get_rate(struct clk * clk)
{
	unsigned long clk_rate = 0;
	size_t i;

	for (i = 0; i < CLK_COUNT; i++)
		if (clk == (struct clk const*)&_dummy_clks[i]) {
			clk_rate = _dummy_clks[i].rate;
			break;
		}

	return clk_rate;
}


int clk_set_rate(struct clk * clk, unsigned long rate)
{
	unsigned long const current_rate = clk_get_rate(clk);
	if (current_rate != rate)
		printk("Warning: cannot change clk: %px rate %lu -> %lu\n",
		       clk, current_rate, rate);

	return 0;
}


int clk_set_rate_exclusive(struct clk * clk,unsigned long rate)
{
	return clk_set_rate(clk, rate);
}


int clk_enable(struct clk * clk) { return 0; }


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
