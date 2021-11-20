/*
 * \brief  Stub driver replacement for irqchip/irq-sun6i-r.c
 * \author Norman Feske
 * \date   2021-11-20
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
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/irqdomain.h>

#include <dt-bindings/interrupt-controller/arm-gic.h>


static struct irq_chip sun6i_r_intc_wakeup_chip = {
	.name             = "sun6i-r-intc",
	.irq_mask         = irq_chip_mask_parent,
	.irq_unmask       = irq_chip_unmask_parent,
	.irq_eoi          = irq_chip_eoi_parent,
	.irq_set_affinity = irq_chip_set_affinity_parent,
	.flags            = IRQCHIP_SET_TYPE_MASKED,
};


static int sun6i_r_intc_domain_translate(struct irq_domain *domain,
                                         struct irq_fwspec *fwspec,
                                         unsigned long *hwirq,
                                         unsigned int *type)
{
	*hwirq = fwspec->param[1];
	*type  = fwspec->param[2] & IRQ_TYPE_SENSE_MASK;

	return 0;
}


static int sun6i_r_intc_domain_alloc(struct irq_domain *domain,
                                     unsigned int virq,
                                     unsigned int nr_irqs, void *arg)
{
	struct irq_fwspec *fwspec = arg;
	struct irq_fwspec gic_fwspec;
	unsigned long hwirq;
	unsigned int type;
	int i, ret;

	(void)sun6i_r_intc_domain_translate(domain, fwspec, &hwirq, &type);

	/* Construct a GIC-compatible fwspec from this fwspec. */
	gic_fwspec = (struct irq_fwspec) {
		.fwnode      = domain->parent->fwnode,
		.param_count = 3,
		.param       = { GIC_SPI, hwirq, type },
	};

	ret = irq_domain_alloc_irqs_parent(domain, virq, nr_irqs, &gic_fwspec);
	if (ret)
		return ret;

	for (i = 0; i < nr_irqs; ++i, ++hwirq, ++virq)
		irq_domain_set_hwirq_and_chip(domain, virq, hwirq,
		                              &sun6i_r_intc_wakeup_chip, 0);

	return 0;
}


static const struct irq_domain_ops sun6i_r_intc_domain_ops = {
	.translate = sun6i_r_intc_domain_translate,
	.alloc     = sun6i_r_intc_domain_alloc,
	.free      = irq_domain_free_irqs_common,
};


static int __init sun6i_a31_r_intc_init(struct device_node *node,
                                        struct device_node *parent)
{
	struct irq_domain *domain, *parent_domain;

	parent_domain = irq_find_host(parent);
	if (!parent_domain) {
		pr_err("%pOF: Failed to obtain parent domain\n", node);
		return -ENXIO;
	}

	domain = irq_domain_add_hierarchy(parent_domain, 0, 0, node,
	                                  &sun6i_r_intc_domain_ops, NULL);
	if (!domain) {
		pr_err("%pOF: Failed to allocate domain\n", node);
		return -ENOMEM;
	}

	return 0;
}


IRQCHIP_DECLARE(sun6i_a31_r_intc, "allwinner,sun6i-a31-r-intc", sun6i_a31_r_intc_init);
