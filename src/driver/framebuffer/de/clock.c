/*
 * \brief  Emulation of the Linux clock interfaces
 * \author Norman Feske
 * \date   2021-11-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <lx_emul/clock.h>

struct de_clock_mapping {
	char const    *client_match_compatible;
	char const    *client_con_id;
	char const    *clock_name;
	unsigned       de_clock_line;
	struct clk    *clk;
	struct clk_hw *hw;
};

enum { NUM_DE_CLOCK_MAPPINGS = 4 };

static struct de_clock_mapping de_clock_mappings[NUM_DE_CLOCK_MAPPINGS] = {
	{ "allwinner,sun50i-a64-de2-mixer-0", "bus", "bus-mixer0", 0, NULL, NULL },
	{ "allwinner,sun50i-a64-de2-mixer-0", "mod", "mixer0",     6, NULL, NULL },
	{ "allwinner,sun50i-a64-de2-mixer-1", "bus", "bus-mixer1", 1, NULL, NULL },
	{ "allwinner,sun50i-a64-de2-mixer-1", "mod", "mixer1",     7, NULL, NULL } };


struct clk *clk_get(struct device *dev, const char *con_id)
{
	struct clk *res = NULL;

	if (!dev || !dev->of_node) {
		printk("Error: clk_get unexpectedly called with no dev!\n");
		lx_emul_trace_and_stop(__func__);
		return NULL;
	}

	if (of_device_is_compatible(dev->of_node, "allwinner,sun50i-a64-de2-clk"))
		res = lx_emul_clock_get(dev->parent->of_node, con_id);

	{
		unsigned i;
		for (i = 0; i < NUM_DE_CLOCK_MAPPINGS; i++) {

			struct de_clock_mapping *mapping = &de_clock_mappings[i];

			if (!of_device_is_compatible(dev->of_node, mapping->client_match_compatible))
				continue;

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


/*
 * Called by drivers/gpu/drm/sun4i/sun4i_dotclock.c during the creation
 * of the tcon pixel clock.
 */

static struct clk    *clk_tcon_pixel_clock;
static struct clk_hw *clk_hw_tcon_pixel_clock;

struct clk * clk_register(struct device * dev,struct clk_hw * hw)
{
	struct clk *res = NULL;

	if (strcmp(hw->init->name, "tcon-data-clock") == 0) {

		if (!clk_tcon_pixel_clock) {

			/*
			 * Apparently, 'sun4i_dclk_create' allocates the 'clk_init_data'
			 * struct of 'hw' at the stack. We copy the structure to avoid
			 * corruption of the 'ops' pointer.
			 */
			struct clk_init_data *copied_init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
			memcpy(copied_init, hw->init, sizeof(struct clk_init_data));
			hw->init = copied_init;

			clk_hw_tcon_pixel_clock = hw;
			clk_tcon_pixel_clock = lx_emul_clock_get(dev->of_node, hw->init->name);
		}
		res = clk_tcon_pixel_clock;
	}

	if (!res)
		lx_emul_trace_and_stop(__func__);

	return res;
}


/*
 * Called by drivers/clk/sunxi-ng/ccu-sun8i-de2.c:345
 */
int of_clk_hw_register(struct device_node * node,struct clk_hw * hw)
{
	if (of_device_is_compatible(node, "allwinner,sun50i-a64-de2-clk")) {

		unsigned i;
		for (i = 0; i < NUM_DE_CLOCK_MAPPINGS; i++) {
			struct de_clock_mapping *mapping = &de_clock_mappings[i];

			if (strcmp(mapping->clock_name, hw->init->name) != 0)
				continue;

			mapping->hw = hw;
		}
	}
	return 0;
}


void clk_unregister(struct clk * clk)
{
	lx_emul_trace_and_stop(__func__);
}


int clk_set_rate_exclusive(struct clk * clk,unsigned long rate)
{
	lx_emul_trace(__func__);
	return 0;
}


int clk_hw_register(struct device * dev,struct clk_hw * hw)
{
	/*
	 * Lookup like done in 'dev_or_parent_of_node()'.
	 */
	struct device_node *np = dev->of_node;
	if (!np) {
		printk("Error: could not lookup device_node for dev %px\n", dev);
		lx_emul_trace_and_stop(__func__);
	}

	return of_clk_hw_register(np, hw);
}


int of_clk_add_hw_provider(struct device_node * np,
                           struct clk_hw * (* get)(struct of_phandle_args * clkspec,void * data),
                           void * data)
{
	lx_emul_trace(__func__);
	return 0;
}


static struct clk_hw *lookup_internal_clk_hw(struct clk *clk)
{
	unsigned i;
	for (i = 0; i < NUM_DE_CLOCK_MAPPINGS; i++) {
		struct de_clock_mapping *mapping = &de_clock_mappings[i];
		if (mapping->clk == clk)
			return mapping->hw;
	}

	if (clk == clk_tcon_pixel_clock)
		return clk_hw_tcon_pixel_clock;

	return NULL;
}


const char *__clk_get_name(const struct clk *clk)
{
	lx_emul_trace(__func__);
	return "unknown-clk";
}


int clk_rate_exclusive_get(struct clk * clk)
{
	lx_emul_trace(__func__);
	return 0;
}


unsigned long clk_get_rate(struct clk * clk)
{
	return lx_emul_clock_get_rate(clk);
}


int clk_set_rate(struct clk * clk,unsigned long rate)
{
	struct clk_hw *hw = lookup_internal_clk_hw(clk);
	if (hw) {
		/* only called for tcon-data-clock */
		unsigned long parent_rate = 275625000; /* tcon-ch0 */
		return hw->init->ops->set_rate(hw, rate, parent_rate);
	}

	if (lx_emul_clock_get_rate(clk) != rate) {
		printk("Warning: cannot change clock rate dynamically "
		       " from %ld to %ld\n", lx_emul_clock_get_rate(clk), rate);
		lx_emul_trace_and_stop(__func__);
	}

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


struct clk * devm_clk_get_enabled(struct device * dev,const char * id)
{
	return NULL;
}
