/*
 * \brief  Post kernel userland activity
 * \author Josef Soentgen
 * \date   2027-07-29
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul/task.h>
#include <lx_user/init.h>
#include <lx_user/io.h>

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched/task.h>
#include <linux/fs.h>
#include <media/media-devnode.h>
#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <uapi/linux/media.h>

#include "lx_user.h"
#include "gui.h"


/* GPIO is 254 */
enum { MEDIA0_MAJOR = 253, };


struct Buffer
{
	unsigned index;

	unsigned char *base;
	size_t         size;

	unsigned vma_flags;
	unsigned vma_pgoff;
};


struct Camera
{
	struct lx_user_config_t config;

	struct Buffer buffer[MAX_BUFFER];

	struct media_v2_topology topology;

	struct inode video_f_inode;
	struct file  video_filp;

	struct inode subdev_f_inode;
	struct file  subdev_filp;

	struct cdev *media0;
	struct cdev *video0;
	struct cdev *v4l_subdev_gc2145;
	struct cdev *v4l_subdev_ov5640;
};


static struct Camera _camera;


struct task_struct *capture_task;
void               *capture_task_args = (void*)&_camera;


struct lx_user_config_t *lx_user_config = &_camera.config;


extern struct cdev *lx_emul_get_cdev(unsigned major, unsigned minor);


/*******************************
 ** Internal camera functions **
 *******************************/

static void dump_topology(struct media_v2_topology *t) __attribute__((unused));
static void dump_topology(struct media_v2_topology *t)
{
	unsigned i;
	struct media_v2_entity *pent;
	struct media_v2_interface *pintf;
	struct media_v2_pad *ppad;
	struct media_v2_link *plin;

	printk("topology_version: %llu\n", t->topology_version);

	printk("num_entities: %u\n", t->num_entities);
	pent = (struct media_v2_entity*)t->ptr_entities;
	for (i = 0; i < t->num_entities; i++) {
		struct media_v2_entity *p = &pent[i];
		printk("[%u] id: 0x%x name: '%s' function: %u flags: 0x%x\n",
		       i, p->id, p->name, p->function, p->flags);
	}
	printk("num_interfaces: %u\n", t->num_interfaces);
	pintf = (struct media_v2_interface*)t->ptr_interfaces;
	for (i = 0; i < t->num_interfaces; i++) {
		struct media_v2_interface *p = &pintf[i];

		printk("[%u] id: 0x%x intf_type: %u flags: 0x%x\n",
		       i, p->id, p->intf_type, p->flags);
	}
	printk("num_pads: %u\n", t->num_pads);
	ppad = (struct media_v2_pad*)t->ptr_pads;
	for (i = 0; i < t->num_pads; i++) {
		struct media_v2_pad *p = &ppad[i];
		printk("[%u] id: 0x%x entity_id: 0x%x flags: 0x%x index: %u\n",
		       i, p->id, p->entity_id, p->flags, p->index);
	}
	printk("num_links: %u\n", t->num_links);
	plin = (struct media_v2_link*)t->ptr_links;
	for (i = 0; i < t->num_links; i++) {
		struct media_v2_link *p = &plin[i];
		printk("[%u] id: 0x%x source_id: 0x%x sink_id: 0x%x flags: 0x%x\n",
		       i, p->id, p->source_id, p->sink_id, p->flags);
	}
}


static int _alloc_topology(struct media_v2_topology *topology)
{
	gfp_t const flags = GFP_KERNEL | __GFP_ZERO;
	topology->ptr_entities = (u64)
		krealloc_array((void*)topology->ptr_entities,
		               topology->num_entities,
		               sizeof(struct media_v2_entity), flags);
	if (!topology->ptr_entities)
		goto err_entities;

	topology->ptr_interfaces = (u64)
		krealloc_array((void*)topology->ptr_interfaces,
		               topology->num_interfaces,
		               sizeof(struct media_v2_interface), flags);
	if (!topology->ptr_interfaces)
		goto err_interfaces;

	topology->ptr_pads = (u64)
		krealloc_array((void*)topology->ptr_pads,
		               topology->num_pads,
		               sizeof(struct media_v2_pad), flags);
	if (!topology->ptr_pads)
		goto err_pads;

	topology->ptr_links = (u64)
		krealloc_array((void*)topology->ptr_links,
		               topology->num_links,
		               sizeof(struct media_v2_link), flags);
	if (!topology->ptr_links)
		goto err_links;

	return 0;

err_links:
	kfree((void*)topology->ptr_pads);
err_pads:
	kfree((void*)topology->ptr_interfaces);
err_interfaces:
	kfree((void*)topology->ptr_entities);
err_entities:
	return -1;
}


static int _query_media_device(struct cdev              *media,
                               struct media_devnode     *mdev,
                               struct media_v2_topology *topology)
{
	struct file media_filp = {
		.private_data = mdev,
	};
	int err;

	err = media->ops->unlocked_ioctl(&media_filp, MEDIA_IOC_G_TOPOLOGY,
	                                  (unsigned long)topology);
	if (err)
		return err;

	if (_alloc_topology(topology))
		return -1;

	err = media->ops->unlocked_ioctl(&media_filp, MEDIA_IOC_G_TOPOLOGY,
	                                  (unsigned long)topology);
	if (err)
		return err;

	return 0;
}


static int _setup_link(struct cdev              *media,
                       struct media_devnode     *mdev,
                       struct media_v2_topology *topology,
                       bool                      front_camera)
{
	struct media_v2_pad *pads = (struct media_v2_pad*)topology->ptr_pads;
	struct file media_filp = {
		.private_data = mdev,
	};
	struct media_link_desc arg;
	int err;

	/* rear camera */
	memset(&arg, 0, sizeof (arg));
	arg.flags = front_camera ? 0 : MEDIA_LNK_FL_ENABLED;
	arg.source.entity = pads[1].entity_id;
	arg.source.index  = pads[1].index;
	arg.sink.entity   = pads[0].entity_id;
	arg.sink.index    = pads[0].index;
	err = media->ops->unlocked_ioctl(&media_filp, MEDIA_IOC_SETUP_LINK,
	                                  (unsigned long)&arg);
	if (err)
		return err;

	/* front camera */
	memset(&arg, 0, sizeof (arg));
	arg.flags = front_camera ? MEDIA_LNK_FL_ENABLED : 0;
	arg.source.entity = pads[2].entity_id;
	arg.source.index  = pads[2].index;
	arg.sink.entity   = pads[0].entity_id;
	arg.sink.index    = pads[0].index;
	err = media->ops->unlocked_ioctl(&media_filp, MEDIA_IOC_SETUP_LINK,
	                                  (unsigned long)&arg);
	if (err)
		return err;

	return 0;
}


static int _setup_subdev_fmt(struct Camera *camera,
                             bool front_camera)
{
	struct cdev *video        = camera->video0;
	struct cdev *video_subdev = front_camera ? camera->v4l_subdev_gc2145
	                                         : camera->v4l_subdev_ov5640;
	int err;

	memset(&camera->subdev_f_inode, 0, sizeof (camera->subdev_f_inode));
	memset(&camera->subdev_filp,    0, sizeof (camera->subdev_filp));

	camera->subdev_f_inode.i_rdev = video_subdev->dev;
	camera->subdev_filp.f_inode   = &camera->subdev_f_inode;

	err = video_subdev->ops->open(NULL, &camera->subdev_filp);
	if (err) {
		printk("Could not open sub-device: %d\n", err);
		return err;
	}

	/* set frame rate */
	{
		struct v4l2_subdev_frame_interval arg;
		memset(&arg, 0, sizeof(arg));
		arg.pad = 0;
		arg.interval.numerator   = 1;
		arg.interval.denominator = camera->config.fps;

		err = video_subdev->ops->unlocked_ioctl(&camera->subdev_filp,
		                                        VIDIOC_SUBDEV_S_FRAME_INTERVAL,
		                                        (unsigned long)&arg);
		if (err) {
			printk("Could not set frame interval: %d\n", err);
			return err;
		}
	}

	{
		struct v4l2_subdev_format arg;
		memset(&arg, 0, sizeof(arg));
		arg.pad   = 0;
		arg.which = V4L2_SUBDEV_FORMAT_ACTIVE;
		arg.format.width  = camera->config.width;
		arg.format.height = camera->config.height;
		arg.format.code   = camera->config.format ? MEDIA_BUS_FMT_SBGGR8_1X8
		                                          : MEDIA_BUS_FMT_UYVY8_2X8;
		arg.format.field  = V4L2_FIELD_ANY;

		err = video->ops->unlocked_ioctl(&camera->subdev_filp,
		                                 VIDIOC_SUBDEV_S_FMT,
		                                 (unsigned long)&arg);
		if (err) {
			printk("Could not set sub-device format: %d\n", err);
			return err;
		}
	}

	// second time around like MP
	{
		struct v4l2_subdev_frame_interval arg;
		memset(&arg, 0, sizeof(arg));
		arg.pad = 0;
		arg.interval.numerator   = 1;
		arg.interval.denominator = camera->config.fps;

		err = video_subdev->ops->unlocked_ioctl(&camera->subdev_filp,
		                                        VIDIOC_SUBDEV_S_FRAME_INTERVAL,
		                                        (unsigned long)&arg);
		if (err) {
			printk("Could not set frame interval: %d\n", err);
			return err;
		}
	}

	return 0;
}


static int _open_video_device(struct Camera *camera)
{
	struct cdev *video = camera->video0;
	int err;

	err = video->ops->open(NULL, &camera->video_filp);
	return err;
}


static int _query_video_device(struct Camera *camera)
{
	struct cdev *video = camera->video0;
	struct v4l2_capability arg;
	int err;

	memset(&arg, 0, sizeof(arg));
	err = video->ops->unlocked_ioctl(&camera->video_filp,
	                                 VIDIOC_QUERYCAP,
	                                 (unsigned long)&arg);
	if (err) {
		printk("Could not query video device: %d\n", err);
		return err;
	}

	return 0;
}


static int _setup_video_fmt(struct Camera *camera)
{
	struct cdev *video = camera->video0;
	struct v4l2_format arg;
	int err;

	memset(&arg, 0, sizeof(arg));
	arg.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	arg.fmt.pix.width       = camera->config.width;
	arg.fmt.pix.height      = camera->config.height;
	arg.fmt.pix.pixelformat = camera->config.format == FMT_SBGRR8 ? V4L2_PIX_FMT_SBGGR8
	                                                              : V4L2_PIX_FMT_YUV420;
	arg.fmt.pix.field       = V4L2_FIELD_ANY;

	err = video->ops->unlocked_ioctl(&camera->video_filp,
	                                 VIDIOC_S_FMT,
	                                 (unsigned long)&arg);
	if (err) {
		printk("Could not query video device: %d\n", err);
		return err;
	}

	return 0;
}


static int _request_buffers(struct Camera *camera)
{
	struct cdev   *video  = camera->video0;
	struct Buffer *buffer = camera->buffer;
	unsigned num_buffer   = camera->config.num_buffer;
	struct v4l2_requestbuffers arg;
	int err;
	unsigned i;

	memset(&arg, 0, sizeof(arg));
	arg.count  = num_buffer; // suni6-csi wants at least 3 buffers
	arg.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	arg.memory = V4L2_MEMORY_MMAP;

	err = video->ops->unlocked_ioctl(&camera->video_filp, VIDIOC_REQBUFS,
	                                 (unsigned long)&arg);
	if (err) {
		printk("Could not request buffers: %d\n", err);
		return err;
	}

	if (arg.count < 2 || arg.count != num_buffer) {
		printk("Insufficient buffer memory, count: %u\n", arg.count);
		return -ENOMEM;
	}

	for (i = 0; i < arg.count; i++) {
		struct v4l2_buffer arg = {
			.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.memory = V4L2_MEMORY_MMAP,
			.index  = i,
		};
		struct vm_area_struct vma;

		err = video->ops->unlocked_ioctl(&camera->video_filp,
		                                 VIDIOC_QUERYBUF,
		                                 (unsigned long)&arg);
		if (err) {
			printk("Could not query buffer %u: %d\n", i, err);
			return err;
		}

		memset(&vma, 0, sizeof(vma));
		vma.vm_pgoff = arg.m.offset >> PAGE_SHIFT;
		vma.vm_flags = VM_SHARED | VM_READ;

		err = video->ops->mmap(&camera->video_filp, &vma);
		if (err) {
			printk("Could not mmap buffer %u\n", i);
			return err;
		}

		buffer[i].index = i;
		buffer[i].base = (unsigned char*)vma.vm_start;
		buffer[i].size = vma.vm_end - vma.vm_start;
		buffer[i].vma_flags = vma.vm_flags;
		buffer[i].vma_pgoff = vma.vm_pgoff;
	}

	return 0;
}


static int _queue_buffers(struct Camera *camera)
{
	struct cdev   *video  = camera->video0;
	struct Buffer *buffer = camera->buffer;
	unsigned num_buffer   = camera->config.num_buffer;
	int err;
	unsigned i;

	for (i = 0; i < num_buffer; i++) {
		struct v4l2_buffer arg = {
			.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.memory = V4L2_MEMORY_MMAP,
			.index  = buffer[i].index,
		};

		err = video->ops->unlocked_ioctl(&camera->video_filp,
		                                 VIDIOC_QBUF,
		                                 (unsigned long)&arg);
		if (err) {
			printk("Could not queue buffer %u: %d\n", i, err);
			return err;
		}
	}

	return 0;
}


static bool _wait_for_media_cdev(void)
{
	int i;

	/* wait for 5 seconds and then bail */
	for (i = 0; i < 10; i++) {
		if (lx_emul_get_cdev(MEDIA0_MAJOR, 0))
			return false;

		msleep(500);
	}

	return true;
}


static int _configure_capture(struct Camera *camera)
{
	struct cdev *media0 = camera->media0;

	bool const front_camera = camera->config.camera == CAMERA_FRONT;

	struct media_devnode *media0_devnode =
		container_of(media0, struct media_devnode, cdev);
	int err;

	memset(&camera->topology, 0, sizeof (camera->topology));
	err = _query_media_device(media0, media0_devnode, &camera->topology);
	if (err) {
		printk("Could not query topology\n");
		return err;
	}

	err = _setup_link(media0, media0_devnode, &camera->topology,
	                 front_camera);
	if (err) {
		printk("Could not enable %s camera: %d\n",
		       front_camera ? "front" : "rear", err);
		return err;
	}

	/* after rear -> front switch open */
	err = _open_video_device(camera);
	if (err) {
		printk("Could not open video device: %d\n", err);
		return err;
	}

	err = _query_video_device(camera);
	if (err) {
		printk("Could not query video device: %d\n", err);
		return err;
	}

	err = _setup_subdev_fmt(camera, front_camera);
	if (err) {
		printk("Could not set sensor format: %d\n", err);
		return err;
	}

	err = _setup_video_fmt(camera);
	if (err) {
		printk("Could not set video format: %d\n", err);
		return err;
	}

	return 0;
}


/**************************
 ** Gui session handling **
 **************************/

static struct genode_gui *create_gui(struct lx_user_config_t *config)
{
	struct genode_gui_args const args = {
		.label  = config->camera == CAMERA_FRONT ? "gc2154" : "ov5640",
		/* use double the width for double buffering */
		.width  = config->rotate ? config->height    : config->width * 2,
		.height = config->rotate ? config->width * 2 : config->height ,
	};
	return genode_gui_create(&args);
}


struct genode_gui_refresh_context
{
	struct Buffer const *buffer;

	unsigned width;
	unsigned height;

	bool view_flip;

	bool convert;
	bool rotate;
	bool gray;
};


static void _rotate_y_as_gray(unsigned char const *y,
                              unsigned int width,
                              unsigned int height,
                              unsigned int *dst)
{
	unsigned w_offset = width;

	unsigned h, w;
	for (w = 0; w < width; w++) {

		unsigned h_offset = 0;
		for (h = 0; h < height; h++) {

			unsigned char const v = *(y + w_offset + h_offset);
			unsigned int  const abgr = 0xff000000u
			                         | (v)
			                         | (v << 8)
			                         | (v << 16);
			*(dst++) = abgr;
			h_offset += width;
		}
		w_offset--;
	}
}


#include "yuv_rgb.h"


static char _convert_buffer[MAX_WIDTH*MAX_HEIGHT * 4 /*abgr*/];


static void _gui_show(struct genode_gui_refresh_context *ctx,
                     unsigned char *dst, size_t size)
{
	struct Buffer *b = ctx->buffer;

	unsigned const int width  = ctx->width;
	unsigned const int height = ctx->height;
	unsigned const int pixels = width * height;

	unsigned char *y = b->base;
	unsigned char *v = y +  (pixels);
	unsigned char *u = v + ((pixels)/4);
	unsigned const int y_stride  = width;
	unsigned const int uv_stride = width/2;

	unsigned int *p = (unsigned int*)dst + (ctx->view_flip * pixels);

	lx_emul_mem_cache_invalidate((void*)b->base, b->size);

	/* fast-path for raw access */
	if (!ctx->convert && !ctx->rotate) {
		memcpy(p, b->base, b->size > size ? size : b->size);
		return;
	}

	/* fast-path for grayish rotate */
	if (ctx->convert && ctx->rotate && ctx->gray) {
		_rotate_y_as_gray(y, width, height, p);
		return;
	}

	if (ctx->convert) {
		yuv420_abgr_std(width, height,
		                y, u, v, y_stride, uv_stride,
		                (unsigned char*)_convert_buffer, width * 4,
		                YCBCR_601);
	} else
		memcpy(_convert_buffer, b->base, b->size > size ? size : b->size);

	if (ctx->rotate) {
		unsigned int *d = (unsigned int*)p;
		unsigned int *s = (unsigned int*)_convert_buffer;

		/*
		 * +---w---+
		 * |   <-- |
		 * |       h
		 * |     ^ |
		 * |     | |
		 * +-------+
		 *
		 * w_offset selects source column and h_offset source row,
		 * copy ccw.
		 */
		unsigned       w_offset       = width;

		unsigned h, w;
		for (w = 0; w < width; w++) {

			unsigned h_offset = 0;
			for (h = 0; h < height; h++) {
				*(d++) = *(s + w_offset + h_offset);
				h_offset += width;
			}
			w_offset--;
		}
	} else
		memcpy(p, _convert_buffer, pixels * 4);
}


static struct genode_gui_view
_gui_set_view(struct genode_gui_refresh_context *ctx)
{
	struct genode_gui_view view = {
		.x      = 0,
		.y      = 0,
		.width  = 0,
		.height = 0,
	};

	/* display first buffer while painting to second */
	view.x = ctx->view_flip ? 0
	                         : ctx->rotate ? 0
	                                       : -ctx->width;
	view.y = ctx->view_flip ? 0
	                         : ctx->rotate ? -ctx->width
	                                       : 0;

	view.width  = ctx->rotate ? ctx->height : ctx->width;
	view.height = ctx->rotate ? ctx->width  : ctx->height;

	return view;
}


static void gui_display_image(struct genode_gui             *gui,
                              struct Buffer           const *b,
                              struct lx_user_config_t const *config,
                              bool                           view_flip)
{
	struct genode_gui_refresh_context ctx = {
		.buffer  = b,
		.width   = config->width,
		.height  = config->height,
		.convert = config->convert,
		.rotate  = config->rotate,
		.gray    = config->gray,
		.view_flip = view_flip,
	};

	genode_gui_swap_view(gui, _gui_set_view, &ctx);

	genode_gui_refresh(gui, _gui_show, &ctx);
}


/****************************************
 ** Camera interface and task handling **
 ****************************************/

static struct Buffer *get_buffer(struct Camera *camera)
{
	struct cdev *video = camera->video0;
	struct v4l2_buffer arg;
	int err;

	memset(&arg, 0, sizeof(arg));
	arg.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	arg.memory = V4L2_MEMORY_MMAP;

	/*
	 * We could set non-blocking but since we are running in
	 * our own task we will simply wait for data to appear.
	 *
	 * filp.f_flags |= O_NONBLOCK;
	 */

	err = video->ops->unlocked_ioctl(&camera->video_filp,
	                                 VIDIOC_DQBUF,
	                                 (unsigned long)&arg);
	if (err) {
		printk("Could not get buffer: %d\n", err);
		return NULL;
	}

	return &camera->buffer[arg.index];
}


static int put_buffer(struct Camera *camera, struct Buffer *b)
{
	struct cdev *video = camera->video0;
	struct v4l2_buffer arg;
	int err;

	memset(&arg, 0, sizeof(arg));
	arg.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	arg.memory = V4L2_MEMORY_MMAP;
	arg.index  = b->index;

	err = video->ops->unlocked_ioctl(&camera->video_filp,
	                                 VIDIOC_QBUF,
	                                 (unsigned long)&arg);
	if (err) {
		printk("Could not put buffer: %d\n", err);
		return err;
	}

	return 0;
}


static int control_camera(struct Camera *camera, bool start)
{
	struct cdev *video = camera->video0;

	enum v4l2_buf_type const arg = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int err;

	err = video->ops->unlocked_ioctl(&camera->video_filp,
	                                 start ? VIDIOC_STREAMON
	                                       : VIDIOC_STREAMOFF,
	                                 (unsigned long)&arg);
	if (err) {
		printk("Could not %s capturing: %d\n", start ? "start"
		                                             : "stop", err);
		return err;
	}
	return 0;
}


static bool setup_camera(struct Camera *camera)
{
	struct cdev *media0;
	struct cdev *video0;
	struct cdev *v4l_subdev_ov5640;
	struct cdev *v4l_subdev_gc2145;

	if (_wait_for_media_cdev()) {
		printk("Timeout while waiting for media device'\n");
		return false;
	}

	media0            = lx_emul_get_cdev(MEDIA0_MAJOR, 0);
	video0            = lx_emul_get_cdev(VIDEO_MAJOR,  0);
	v4l_subdev_ov5640 = lx_emul_get_cdev(VIDEO_MAJOR,  1);
	v4l_subdev_gc2145 = lx_emul_get_cdev(VIDEO_MAJOR,  2);

	if (!media0 || !video0 || !v4l_subdev_gc2145 || !v4l_subdev_ov5640) {
		printk("Could not acquire video devices\n");
		return false;
	}

	camera->media0            = media0;
	camera->video0            = video0;
	camera->v4l_subdev_gc2145 = v4l_subdev_gc2145;
	camera->v4l_subdev_ov5640 = v4l_subdev_ov5640;

	/* prepare ioctl arguments */
	memset(&camera->video_f_inode, 0, sizeof (camera->video_f_inode));
	memset(&camera->video_filp,    0, sizeof (camera->video_filp));
	camera->video_f_inode.i_rdev = video0->dev;
	camera->video_filp.f_inode   = &camera->video_f_inode;

	if (_configure_capture(camera))
		return false;

	if (_request_buffers(camera)) {
		printk("Could not request buffers\n");
		return false;
	}

	if (_queue_buffers(camera)) {
		printk("Could not queue buffers\n");
		return false;
	}

	return true;
}


static void sleep_forever(void)
{
	/* parent exit? */
	__set_current_state(TASK_DEAD);
	schedule();
	BUG();
}


static int capture_task_function(void *p)
{
	struct Camera *camera = (struct Camera*)p;
	struct genode_gui *gui;

	unsigned const skip_frames = camera->config.skip_frames;
	unsigned skip_count;
	bool view_flip;

	if (!camera->config.valid) {
		printk("Camera configuration invalid\n");
		sleep_forever();
	}

	if (!setup_camera(camera))
		sleep_forever();

	gui = create_gui(&camera->config);
	if (!gui) {
		printk("Could not create Gui session\n");
		sleep_forever();
	}

	if (control_camera(camera, true))
		BUG();

	view_flip = true;
	skip_count = 0;
	while (true) {
		/* get_buffer will block when no buffer is available */
		struct Buffer *b = get_buffer(camera);
		if (!b)
			break;

		if (skip_count >= skip_frames) {
			gui_display_image(gui, b, &camera->config, view_flip);
			view_flip = view_flip ? false : true;
			skip_count = 0;
		}
		skip_count++;

		put_buffer(camera, b);
	}

	(void)control_camera(camera, false);

	sleep_forever();
	/* never reached */
	return 0;
}


void lx_user_handle_io(void) { }


void lx_user_init(void)
{
	int pid = kernel_thread(capture_task_function, capture_task_args,
	                        CLONE_FS | CLONE_FILES);
	capture_task = find_task_by_pid_ns(pid, NULL);
}
