/*
 * \brief  Emulation of the Linux input subsystem
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <genode_c_api/event.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/slab.h>


struct input_dev * devm_input_allocate_device(struct device * dev)
{
	struct input_dev *input = kzalloc(sizeof(struct input_dev), GFP_KERNEL);

	input->dev.parent = dev;

	/* space for buffering input values that belong to one multi-touch frame */
	input->max_vals = 256;
	input->vals = kzalloc(sizeof(*input->vals)*input->max_vals, GFP_KERNEL);

	return input;
}


void input_alloc_absinfo(struct input_dev * dev)
{
	dev->absinfo = kzalloc(sizeof(*dev->absinfo), GFP_KERNEL);
}


int input_mt_init_slots(struct input_dev * dev,unsigned int num_slots,unsigned int flags)
{
	dev->mt = kzalloc(sizeof(*dev->mt) + num_slots*sizeof(input_mt_init_slots), GFP_KERNEL);

	dev->mt->num_slots = num_slots;

	return 0;
}


void input_set_capability(struct input_dev * dev,unsigned int type,unsigned int code)
{
	lx_emul_trace(__func__);

	switch (type) {
	case EV_KEY:
		__set_bit(code, dev->keybit);
		break;

	case EV_ABS:
		input_alloc_absinfo(dev);
		if (!dev->absinfo)
			return;

		__set_bit(code, dev->absbit);
		break;

	default:
		printk("%s: unknown type %u (code %u)\n", __func__, type, code);
		return;
	}

	__set_bit(type, dev->evbit);
}


void input_copy_abs(struct input_dev *dst, unsigned int dst_axis,
            const struct input_dev *src, unsigned int src_axis)
{
	/* src must have EV_ABS and src_axis set */
	if (WARN_ON(!(test_bit(EV_ABS, src->evbit) &&
				  test_bit(src_axis, src->absbit))))
		return;

	if (!src->absinfo)
		return;

	input_set_capability(dst, EV_ABS, dst_axis);
	if (!dst->absinfo)
		return;

	dst->absinfo[dst_axis] = src->absinfo[src_axis];
}


int input_register_device(struct input_dev * dev)
{
	return 0;
}


void input_event(struct input_dev * dev,unsigned int type,unsigned int code,int value)
{
	if (dev->num_vals == dev->max_vals) {
		printk("input_event: overflow, dropping event\n");
		return;
	}

	/* ignore events other than absolute motion */
	if (type != EV_ABS)
		return;

	{
		struct input_value *val = &dev->vals[dev->num_vals];

		val->type  = type;
		val->code  = code;
		val->value = value;

		dev->num_vals++;
	}
}


bool input_mt_report_slot_state(struct input_dev * dev,unsigned int tool_type,bool active)
{
	return false;
}


struct genode_event_generator_ctx
{
	struct input_dev *input_dev;
};


static void event_generator(struct genode_event_generator_ctx *ctx,
                            struct genode_event_submit *submit)
{
	struct input_dev *dev = ctx->input_dev;
	int i;
	int curr_slot = -1;

	/*
	 * input_mt_slot->key is used as state to distinguish press, move, release
	 *
	 * The 'present' flag is kept across calls of 'input_mt_sync_frame'.
	 * The 'changed' and 'updated' flags are used only within one call.
	 */
	enum {
		MT_KEY_CHANGED_FLAG = 1,   /* at least one value changed */
		MT_KEY_PRESENT_FLAG = 2,   /* finger is touching the screen */
		MT_KEY_UPDATED_FLAG = 4,   /* finger is featured in current frame */
	};

	for (i = 0; i < dev->mt->num_slots; i++) {
		dev->mt->slots[i].key &= ~MT_KEY_CHANGED_FLAG;
		dev->mt->slots[i].key &= ~MT_KEY_UPDATED_FLAG;
	}

	/*
	 * Update slots from captured input values
	 */
	for (i = 0; i < dev->num_vals; i++) {

		struct input_value const * const val = &dev->vals[i];

		if (val->type != EV_ABS)
			continue;

		/* start of multi-touch frame */
		if (val->code == ABS_MT_SLOT)
			curr_slot = val->value;

		if (curr_slot >= 0 && curr_slot < dev->mt->num_slots) {

			struct input_mt_slot *slot = &dev->mt->slots[curr_slot];

			if (val->code >= ABS_MT_FIRST && val->code <= ABS_MT_TOOL_Y) {

				int *abs = &slot->abs[val->code - ABS_MT_FIRST];

				slot->key |= MT_KEY_UPDATED_FLAG;

				/*
				 * Update slot info. Set 'frame' only on changes. This way,
				 * repeated events (with all values unchanged) are ignored.
				 */
				if (*abs != val->value) {
					*abs = val->value;
					slot->key |= MT_KEY_CHANGED_FLAG;
				}

				slot->abs[val->code - ABS_MT_FIRST] = val->value;
			}
		}
	}

	/*
	 * Detect press, release, motion
	 */
	for (i = 0; i < dev->mt->num_slots; i++) {

		struct input_mt_slot *slot = &dev->mt->slots[i];

		bool const present = slot->key & MT_KEY_PRESENT_FLAG;
		bool const changed = slot->key & MT_KEY_CHANGED_FLAG;
		bool const updated = slot->key & MT_KEY_UPDATED_FLAG;

		bool const new_finger   = !present && updated;
		bool const moved_finger = present && updated && changed;

		if (new_finger || moved_finger) {

			struct genode_event_touch_args args = {
				.finger = i,
				.xpos   = slot->abs[ABS_MT_POSITION_X  - ABS_MT_FIRST],
				.ypos   = slot->abs[ABS_MT_POSITION_Y  - ABS_MT_FIRST],
				.width  = slot->abs[ABS_MT_WIDTH_MAJOR - ABS_MT_FIRST]
			};

			submit->touch(submit, &args);

			slot->key |= MT_KEY_PRESENT_FLAG;
		}

		if (present && !updated) {
			submit->touch_release(submit, i);
			slot->key = 0;
		}
	}

	/* capture a new batch of events */
	dev->num_vals = 0;
}


void input_mt_sync_frame(struct input_dev *dev)
{
	/* create Genode event session on first call */
	if (!dev->grab) {
		struct genode_event_args args = { .label = dev->phys };

		dev->grab = (struct input_handle *)genode_event_create(&args);
	}

	/* generate Genode events */
	{
		struct genode_event_generator_ctx ctx = { .input_dev = dev };

		struct genode_event *genode_event = (struct genode_event *)dev->grab;

		genode_event_generate(genode_event, &event_generator, &ctx);
	}
}
