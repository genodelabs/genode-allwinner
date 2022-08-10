/*
 * \brief  PIO stub driver
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/irqdomain.h>
#include <linux/gpio/driver.h>
#include <linux/pinctrl/pinctrl.h>

#include <lx_emul/pin.h>


/*
 * Originally defined in drivers/pinctrl/sunxi/pinctrl-sunxi.h
 */

struct sunxi_pinctrl {
	struct irq_domain *domain;
	struct gpio_chip  *chip;

	/*
	 * The 'global_irq' number is merely used to mimick a GIC interrupt
	 * whenever a GPIO pin interrupt occurs.
	 */
	int global_irq;
};


enum {
	PINS_PER_BANK = 32,
	NUM_BANKS     = 16,
	NUM_PINS      = NUM_BANKS*PINS_PER_BANK,
};


static void sunxi_pinctrl_pin_name(char *buf, size_t len, int pin_number)
{
	int const bank = pin_number / PINS_PER_BANK;
	int const pin  = pin_number % PINS_PER_BANK;

	if (len == 0)
		return;

	snprintf(buf, sizeof(len), "P%c%d", 'A' + bank, pin);
	buf[len - 1] = 0;
}


static int sunxi_pinctrl_irq_of_xlate(struct irq_domain *d,
                                      struct device_node *node,
                                      const u32 *intspec,
                                      unsigned int intsize,
                                      unsigned long *out_hwirq,
                                      unsigned int *out_type)
{
	int base_bank = intspec[0];
	int pin       = intspec[1];

	*out_hwirq = base_bank*PINS_PER_BANK + pin;
	*out_type  = intspec[2];

	return 0;
}


static const struct irq_domain_ops sunxi_pinctrl_irq_domain_ops = {
	.xlate = sunxi_pinctrl_irq_of_xlate,
};


static int sunxi_pinctrl_gpio_of_xlate(struct gpio_chip *gc,
                                       const struct of_phandle_args *gpiospec,
                                       u32 *flags)
{
	int const bank   = gpiospec->args[0];
	int const pin    = gpiospec->args[1];
	int const number = bank*PINS_PER_BANK + pin;

	char name[16];
	sunxi_pinctrl_pin_name(name, sizeof(name), number);

	if (flags)
		*flags = gpiospec->args[2];

	return number;
}


static int sunxi_pinctrl_gpio_set_config(struct gpio_chip *gc, unsigned int offset,
                                         unsigned long config)
{
	return 0;
}


static void sunxi_pinctrl_gpio_set(struct gpio_chip *chip,
                                   unsigned offset, int value)
{
	char name[16];
	sunxi_pinctrl_pin_name(name, sizeof(name), offset);

	lx_emul_pin_control(name, value);
}


static int sunxi_pinctrl_gpio_direction_output(struct gpio_chip *chip,
                                               unsigned offset, int value)
{
	sunxi_pinctrl_gpio_set(chip, offset, value);
	return 0;
}


static int sunxi_pinctrl_gpio_get(struct gpio_chip *chip,
                                  unsigned offset)
{
	char name[16];
	sunxi_pinctrl_pin_name(name, sizeof(name), offset);

	/*
	 * Allow the time-multiplexed operation of selected pins as both input and
	 * output.
	 */

	/* CSI I2C driven via bit banging */
	if (strcmp(name, "PE12") == 0 || strcmp(name, "PE13") == 0)
		return lx_emul_pin_sense(name);

	/* reset line of the PinePhone's rear camera */
	if (strcmp(name, "PD3" ) == 0)
		return lx_emul_pin_sense(name);

	return 0;
}


static int sunxi_pinctrl_gpio_direction_input(struct gpio_chip *chip,
                                              unsigned offset)
{
	return sunxi_pinctrl_gpio_get(chip, offset);
}


static void sunxi_pinctrl_irq_ack(struct irq_data *pin_irq_data)
{
	lx_emul_pin_irq_ack(pin_irq_data->hwirq);
}


static void sunxi_pinctrl_irq_unmask(struct irq_data *pin_irq_data)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(pin_irq_data);

	char name[16];
	sunxi_pinctrl_pin_name(name, sizeof(name), pin_irq_data->hwirq);

	{
		struct irq_data *gic_irq_data = irq_get_irq_data(pctl->global_irq);

		if (gic_irq_data)
			lx_emul_pin_irq_unmask(gic_irq_data->hwirq, pin_irq_data->hwirq, name);
	}
}


static struct irq_chip sunxi_pinctrl_edge_irq_chip = {
	.name       = "sunxi_pio_edge",
	.irq_ack    = sunxi_pinctrl_irq_ack,
	.irq_unmask = sunxi_pinctrl_irq_unmask,
};


static void sunxi_pinctrl_create_irq_mapping(struct sunxi_pinctrl *pctl, int pin_number)
{
	int const irqno = irq_create_mapping(pctl->domain, pin_number);

	irq_set_chip_and_handler(irqno, &sunxi_pinctrl_edge_irq_chip,
	                         handle_edge_irq);
	irq_set_chip_data(irqno, pctl);
}


static void sunxi_pinctrl_irq_handler(struct irq_desc *desc)
{
	struct irq_chip      *chip = irq_desc_get_chip(desc);
	struct sunxi_pinctrl *pctl = irq_desc_get_handler_data(desc);

	chained_irq_enter(chip, desc);

	{
		int pin_irq = irq_find_mapping(pctl->domain, lx_emul_pin_last_irq());

		generic_handle_irq(pin_irq);
	}

	chained_irq_exit(chip, desc);
}


static int a64_pinctrl_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct sunxi_pinctrl *pctl;

	pctl = devm_kzalloc(&pdev->dev, sizeof(*pctl), GFP_KERNEL);
	if (!pctl)
		return -ENOMEM;

	platform_set_drvdata(pdev, pctl);

	{
		pctl->chip = devm_kzalloc(&pdev->dev, sizeof(*pctl->chip), GFP_KERNEL);
		if (!pctl->chip)
			return -ENOMEM;

		pctl->chip->owner           = THIS_MODULE;
		pctl->chip->request         = gpiochip_generic_request;
		pctl->chip->free            = gpiochip_generic_free;
		pctl->chip->set_config      = sunxi_pinctrl_gpio_set_config;
		pctl->chip->set             = sunxi_pinctrl_gpio_set;
		pctl->chip->get             = sunxi_pinctrl_gpio_get;
		pctl->chip->of_xlate        = sunxi_pinctrl_gpio_of_xlate;
		pctl->chip->of_gpio_n_cells = 3;
		pctl->chip->ngpio           = NUM_PINS;
		pctl->chip->label           = dev_name(&pdev->dev);
		pctl->chip->parent          = &pdev->dev;

		pctl->chip->direction_output = sunxi_pinctrl_gpio_direction_output;
		pctl->chip->direction_input  = sunxi_pinctrl_gpio_direction_input;

		{
			int ret;
			ret = gpiochip_add_data(pctl->chip, pctl);
			if (ret)
				return ret;
		}
	}

	pctl->domain = irq_domain_add_linear(node,
	                                     NUM_PINS,
	                                     &sunxi_pinctrl_irq_domain_ops,
	                                     pctl);

	sunxi_pinctrl_create_irq_mapping(pctl, 228); /* PH4 (touchscreen) */

	/*
	 * Request GIC interrupt to be triggered whenever a GPIO pin interrupt
	 * occurs.
	 */
	pctl->global_irq = platform_get_irq(pdev, 0);

	irq_set_chained_handler_and_data(pctl->global_irq,
	                                 sunxi_pinctrl_irq_handler, pctl);

	return 0;
}


static const struct of_device_id a64_pinctrl_match[] = {
	{ .compatible = "allwinner,sun50i-a64-pinctrl", },
	{ }
};


static struct platform_driver a64_pinctrl_driver = {
	.probe  = a64_pinctrl_probe,
	.driver = {
		.name           = "sun50i-a64-pinctrl",
		.of_match_table = a64_pinctrl_match,
	},
};
builtin_platform_driver(a64_pinctrl_driver);

