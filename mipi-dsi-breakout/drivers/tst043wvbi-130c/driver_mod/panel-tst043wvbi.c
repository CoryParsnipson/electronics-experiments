// SPDX-License-Identifier: GPL-2.0

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

#include <video/mipi_display.h>

struct panel_desc {
	const struct drm_display_mode *display_mode;
	unsigned long mode_flags;
	enum mipi_dsi_pixel_format format;
	unsigned int lanes;
	void (*init_sequence)(struct mipi_dsi_multi_context *ctx);
};

struct ili9806e_panel {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	struct regulator_bulk_data supplies[2];
	const struct panel_desc *desc;
	enum drm_panel_orientation orientation;
};

static const char * const regulator_names[] = {
	"vdd",
	"vccio",
};

static inline struct ili9806e_panel *to_ili9806e_panel(struct drm_panel *panel)
{
	return container_of(panel, struct ili9806e_panel, panel);
}

static int ili9806e_power_on(struct ili9806e_panel *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	int ret;

	gpiod_set_value(ctx->reset_gpio, 1);

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(&dsi->dev, "regulator bulk enable failed: %d\n", ret);
		return ret;
	}

	usleep_range(10000, 20000);
	gpiod_set_value(ctx->reset_gpio, 0);
	usleep_range(10000, 20000);

	return 0;
}

static int ili9806e_power_off(struct ili9806e_panel *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	int ret;

	gpiod_set_value(ctx->reset_gpio, 1);

	ret = regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret)
		dev_err(&dsi->dev, "regulator bulk disable failed: %d\n", ret);

	return ret;
}

static int ili9806e_on(struct ili9806e_panel *ili9806e)
{
	struct mipi_dsi_multi_context ctx = { .dsi = ili9806e->dsi };

	if (ili9806e->desc->init_sequence)
		ili9806e->desc->init_sequence(&ctx);

	mipi_dsi_dcs_exit_sleep_mode_multi(&ctx);
	mipi_dsi_msleep(&ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&ctx);

	return ctx.accum_err;
}

static int ili9806e_off(struct ili9806e_panel *panel)
{
	struct mipi_dsi_multi_context ctx = { .dsi = panel->dsi };

	mipi_dsi_dcs_set_display_off_multi(&ctx);
	mipi_dsi_dcs_enter_sleep_mode_multi(&ctx);
	mipi_dsi_msleep(&ctx, 120);

	return ctx.accum_err;
}

static int ili9806e_prepare(struct drm_panel *panel)
{
	struct ili9806e_panel *ctx = to_ili9806e_panel(panel);
	int ret;

	ret = ili9806e_power_on(ctx);
	if (ret < 0)
		return ret;

	ret = ili9806e_on(ctx);
	if (ret < 0) {
		ili9806e_power_off(ctx);
		return ret;
	}

	return 0;
}

static int ili9806e_unprepare(struct drm_panel *panel)
{
	struct ili9806e_panel *ctx = to_ili9806e_panel(panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	int ret;

	ili9806e_off(ctx);

	ret = ili9806e_power_off(ctx);
	if (ret < 0)
		dev_err(&dsi->dev, "power off failed: %d\n", ret);

	return ret;
}

static int ili9806e_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct ili9806e_panel *ctx = to_ili9806e_panel(panel);
	const struct drm_display_mode *mode = ctx->desc->display_mode;

	return drm_connector_helper_get_modes_fixed(connector, mode);
}

static enum drm_panel_orientation ili9806e_get_orientation(struct drm_panel *panel)
{
	struct ili9806e_panel *ctx = to_ili9806e_panel(panel);

	return ctx->orientation;
}

static const struct drm_panel_funcs ili9806e_funcs = {
	.prepare = ili9806e_prepare,
	.unprepare = ili9806e_unprepare,
	.get_modes = ili9806e_get_modes,
	.get_orientation = ili9806e_get_orientation,
};

static int ili9806e_dsi_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct ili9806e_panel *ctx;
	int i, ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->desc = device_get_match_data(dev);

	for (i = 0; i < ARRAY_SIZE(ctx->supplies); i++)
		ctx->supplies[i].supply = regulator_names[i];

	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies),
				      ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	mipi_dsi_set_drvdata(dsi, ctx);
	ctx->dsi = dsi;

	dsi->mode_flags = ctx->desc->mode_flags;
	dsi->format = ctx->desc->format;
	dsi->lanes = ctx->desc->lanes;

	drm_panel_init(&ctx->panel, dev, &ili9806e_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = of_drm_get_panel_orientation(dev->of_node, &ctx->orientation);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get orientation\n");

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	ctx->panel.prepare_prev_first = true;
	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void ili9806e_dsi_remove(struct mipi_dsi_device *dsi)
{
	struct ili9806e_panel *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
}

static void com35h3p70ulc_init(struct mipi_dsi_multi_context *ctx)
{
	/* Switch to page 1 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x01);
	/* Interface Settings */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x18);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x21, 0x01);
	/* Panel Settings */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x30, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x60, 0x0d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x61, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x62, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x63, 0x09);
	/* Power Control */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x30);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x41, 0x44);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x42, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x43, 0x89);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x44, 0x8e);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x45, 0xd9);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x46, 0x33);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x47, 0x33);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x50, 0x90);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x90);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x56, 0x00);
	/* Gamma Settings */
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa0, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa1, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa2, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa3, 0x0f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa4, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa5, 0x0d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa6, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa7, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa8, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa9, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xaa, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xab, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xac, 0x12);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xad, 0x28);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xae, 0x20);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xaf, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc1, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc2, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc3, 0x0f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc4, 0x09);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc5, 0x0d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc6, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc7, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc8, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc9, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xca, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcb, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcc, 0x0f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcd, 0x21);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xce, 0x17);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcf, 0x0a);

	/* Switch to page 7 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x07);
	/* Power Control */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x18, 0x1d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x17, 0x32);

	/* Switch to page 6 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x06);
	/* GIP settings */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x00, 0x20);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x01, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x02, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x03, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x04, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x05, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x88);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x07, 0x04);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x09, 0x80);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0a, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0b, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0c, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0d, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0e, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x10, 0x55);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x11, 0x50);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x12, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x13, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x14, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x15, 0x43);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x16, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x17, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x19, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1b, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1c, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x20, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x21, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x22, 0x45);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x23, 0x67);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x24, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x25, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x26, 0x45);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x27, 0x67);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x30, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x32, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x33, 0x88);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x34, 0xaa);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x35, 0xbb);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x36, 0x66);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x37, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x38, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x39, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3a, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3b, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3c, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3d, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3e, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3f, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x12);

	/* Switch to page 0 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x00);
	/* Interface Pixel format */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3a, 0x60);
};

static const struct drm_display_mode com35h3p70ulc_default_mode = {
	.clock = 22400,
	.hdisplay = 480,
	.hsync_start = 480 + 16,
	.hsync_end = 480 + 16 + 16,
	.htotal = 480 + 16 + 16 + 16,
	.vdisplay = 640,
	.vsync_start = 640 + 52,
	.vsync_end = 640 + 52 + 4,
	.vtotal = 640 + 52 + 4 + 16,
	.width_mm = 53,
	.height_mm = 71,
};

static const struct panel_desc com35h3p70ulc_desc = {
	.init_sequence = com35h3p70ulc_init,
	.display_mode = &com35h3p70ulc_default_mode,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
		      MIPI_DSI_MODE_LPM,
	.format = MIPI_DSI_FMT_RGB888,
	.lanes = 2,
};

static void dmt028vghmcmi_1d_init(struct mipi_dsi_multi_context *ctx)
{
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x21, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x30, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x60, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x61, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x62, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x63, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x16);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x41, 0x44);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x42, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x43, 0x83);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x44, 0x89);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x45, 0x8a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x46, 0x44);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x47, 0x44);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x50, 0x78);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x78);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x52, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x6c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x54, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x55, 0x6c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x56, 0x00);
	/* Gamma settings */
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa0, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa1, 0x09);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa2, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa3, 0x09);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa4, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa5, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa6, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa7, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa8, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa9, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xaa, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xab, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xac, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xad, 0x19);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xae, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xaf, 0x00);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc1, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc2, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc3, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc4, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc5, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc6, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc7, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc8, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc9, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xca, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcb, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcc, 0x0d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcd, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xce, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xcf, 0x00);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x17, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x18, 0x1d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x02, 0x77);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe1, 0x79);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x13);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x06);
	/* GIP 0 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x00, 0x21);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x01, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x02, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x03, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x04, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x05, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x98);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x07, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x09, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0a, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0b, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0c, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0d, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0e, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x10, 0xf7);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x11, 0xf0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x12, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x13, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x14, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x15, 0xc0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x16, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x17, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x19, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1b, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1c, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1d, 0x00);
	/* GIP 1 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x20, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x21, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x22, 0x44);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x23, 0x67);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x24, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x25, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x26, 0x45);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x27, 0x67);
	/* GIP 2 */
	mipi_dsi_dcs_write_seq_multi(ctx, 0x30, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x32, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x33, 0xbc);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x34, 0xad);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x35, 0xda);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x36, 0xcb);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x37, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x38, 0x55);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x39, 0x76);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3a, 0x67);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3b, 0x88);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3c, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3d, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3e, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3f, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x22);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x52, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x54, 0x13);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x00);
};

static const struct drm_display_mode dmt028vghmcmi_1d_default_mode = {
	.clock		= 22000,

	.hdisplay	= 480,
	.hsync_start	= 480 + 20,
	.hsync_end	= 480 + 20 + 4,
	.htotal		= 480 + 20 + 4 + 10,

	.vdisplay	= 640,
	.vsync_start	= 640 + 40,
	.vsync_end	= 640 + 40 + 4,
	.vtotal		= 640 + 40 + 4 + 20,

	.width_mm	= 53,
	.height_mm	= 79,

	.flags		= DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC,
	.type		= DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const struct panel_desc dmt028vghmcmi_1d_desc = {
	.init_sequence = dmt028vghmcmi_1d_init,
	.display_mode = &dmt028vghmcmi_1d_default_mode,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
		      MIPI_DSI_MODE_LPM | MIPI_DSI_CLOCK_NON_CONTINUOUS,
	.format = MIPI_DSI_FMT_RGB888,
	.lanes = 2,
};

static void tst043wvbi_init(struct mipi_dsi_multi_context *ctx) {
	// Page 1 Commands
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x01);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x10);  // output SDA
	mipi_dsi_dcs_write_seq_multi(ctx, 0x21, 0x01);  // DE = 1 Active
	mipi_dsi_dcs_write_seq_multi(ctx, 0x30, 0x02);  // 480 x 800
	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x00);  // 2dot inversion
	mipi_dsi_dcs_write_seq_multi(ctx, 0x60, 0x07);  // SDTI
	mipi_dsi_dcs_write_seq_multi(ctx, 0x61, 0x06);  // CRTI
	mipi_dsi_dcs_write_seq_multi(ctx, 0x62, 0x06);  // EQTI
	mipi_dsi_dcs_write_seq_multi(ctx, 0x63, 0x04);  // PCTI
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x14);  // BT +2.5/-2.5 pump for DDVDH-L
	mipi_dsi_dcs_write_seq_multi(ctx, 0x41, 0x33);  // DVDDH DVDDL clamp
	mipi_dsi_dcs_write_seq_multi(ctx, 0x42, 0x01);  // VGH/VGL 31.37ma 32.7ma 0.03
	mipi_dsi_dcs_write_seq_multi(ctx, 0x43, 0x09);  // VGH/VGL 13 -9 5 -3.8 1.6 -1.5 -2.75
	mipi_dsi_dcs_write_seq_multi(ctx, 0x44, 0x0A);  // VGH/VGL
	mipi_dsi_dcs_write_seq_multi(ctx, 0x45, 0x0A);  // VGH/VGL  -10V
	mipi_dsi_dcs_write_seq_multi(ctx, 0x50, 0x78);  // VGMP
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x78);  // VGMN
	mipi_dsi_dcs_write_seq_multi(ctx, 0x52, 0x00);  // Flicker
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x3B);  // Flicker4F
	mipi_dsi_dcs_write_seq_multi(ctx, 0x57, 0x50);

	// Positive Gamma
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA0, 0x00);  // Gamma   0 / 255
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA1, 0x13);  // Gamma   4 / 251
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA2, 0x19);  // Gamma   8 / 247
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA3, 0x0C);  // Gamma  16 / 239
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA4, 0x06);  // Gamma  24 / 231
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA5, 0x0A);  // Gamma  52 / 203
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA6, 0x06);  // Gamma  80 / 175
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA7, 0x04);  // Gamma 108 / 147
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA8, 0x09);  // Gamma 147 / 108
	mipi_dsi_dcs_write_seq_multi(ctx, 0xA9, 0x08);  // Gamma 175 /  80 
	mipi_dsi_dcs_write_seq_multi(ctx, 0xAA, 0x12);  // Gamma 203 /  52
	mipi_dsi_dcs_write_seq_multi(ctx, 0xAB, 0x06);  // Gamma 231 /  24
	mipi_dsi_dcs_write_seq_multi(ctx, 0xAC, 0x0E);  // Gamma 239 /  16
	mipi_dsi_dcs_write_seq_multi(ctx, 0xAD, 0x0E);  // Gamma 247 /   8
	mipi_dsi_dcs_write_seq_multi(ctx, 0xAE, 0x09);  // Gamma 251 /   4
	mipi_dsi_dcs_write_seq_multi(ctx, 0xAF, 0x00);  // Gamma 255 /   0

	// Negative Gamma
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC0, 0x00);  // Gamma 0
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC1, 0x0D);  // Gamma 4
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC2, 0x18);  // Gamma 8
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC3, 0x0D);  // Gamma 16
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC4, 0x06);  // Gamma 24
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC5, 0x09);  // Gamma 52
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC6, 0x07);  // Gamma 80
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC7, 0x05);  // Gamma 108
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC8, 0x08);  // Gamma 147
	mipi_dsi_dcs_write_seq_multi(ctx, 0xC9, 0x0E);  // Gamma 175
	mipi_dsi_dcs_write_seq_multi(ctx, 0xCA, 0x12);  // Gamma 203
	mipi_dsi_dcs_write_seq_multi(ctx, 0xCB, 0x09);  // Gamma 231
	mipi_dsi_dcs_write_seq_multi(ctx, 0xCC, 0x0E);  // Gamma 239
	mipi_dsi_dcs_write_seq_multi(ctx, 0xCD, 0x0E);  // Gamma 247
	mipi_dsi_dcs_write_seq_multi(ctx, 0xCE, 0x08);  // Gamma 251
	mipi_dsi_dcs_write_seq_multi(ctx, 0xCF, 0x00);  // Gamma 255

	// Page 7 Commands
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x07);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x17, 0x32);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x18, 0x1D);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x26, 0xB2);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x02, 0x77);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xE1, 0x79);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xB3, 0x10);

	// Page 6 Commands
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x06);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x00, 0x20);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x01, 0x04);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x02, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x03, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x04, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x05, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x88);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x07, 0x04);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x09, 0x90);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0A, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0B, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0C, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0D, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0E, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0F, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x10, 0x55);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x11, 0x53);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x12, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x13, 0x0D);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x14, 0x0D);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x15, 0x43);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x16, 0x0B);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x17, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x19, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1A, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1B, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1C, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x1D, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x20, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x21, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x22, 0x45);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x23, 0x67);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x24, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x25, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x26, 0x45);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x27, 0x67);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x30, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x32, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x33, 0xAA);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x34, 0xBB);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x35, 0x66);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x36, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x37, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x38, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x39, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3A, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3B, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3C, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3D, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3E, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3F, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x52, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x12); // VGLO refer VGL_REG
	mipi_dsi_dcs_write_seq_multi(ctx, 0x54, 0x13);

	// Page 0 Commands
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0xff, 0x98, 0x06, 0x04, 0x00);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x21);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x36, 0x08);
};

static const struct drm_display_mode tst043wvbi_default_mode = {
	.clock		= 22000,

	.hdisplay	= 480,
	.hsync_start	= 480 + 20,
	.hsync_end	= 480 + 20 + 4,
	.htotal		= 480 + 20 + 4 + 10,

	.vdisplay	= 800,
	.vsync_start	= 800 + 40,
	.vsync_end	= 800 + 40 + 4,
	.vtotal		= 800 + 40 + 4 + 20,

	.width_mm	= 56,
	.height_mm	= 94,

	.flags		= DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC,
	.type		= DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const struct panel_desc tst043wvbi_desc = {
	.init_sequence = tst043wvbi_init,
	.display_mode = &tst043wvbi_default_mode,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
		      MIPI_DSI_MODE_LPM | MIPI_DSI_CLOCK_NON_CONTINUOUS,
	.format = MIPI_DSI_FMT_RGB888,
};

static const struct of_device_id ili9806e_of_match[] = {
	{ .compatible = "densitron,dmt028vghmcmi-1d", .data = &dmt028vghmcmi_1d_desc },
	{ .compatible = "ortustech,com35h3p70ulc", .data = &com35h3p70ulc_desc },
	{ .compatible = "dongguan,tst043wvbi", .data = &tst043wvbi_desc},
	{ }
};
MODULE_DEVICE_TABLE(of, ili9806e_of_match);

static struct mipi_dsi_driver ili9806e_dsi_driver = {
	.driver = {
		.name = "ili9806e-dsi",
		.of_match_table = ili9806e_of_match,
	},
	.probe = ili9806e_dsi_probe,
	.remove = ili9806e_dsi_remove,
};
module_mipi_dsi_driver(ili9806e_dsi_driver);

MODULE_AUTHOR("Gunnar Dibbern <gunnar.dibbern@lht.dlh.de>");
MODULE_AUTHOR("Michael Walle <mwalle@kernel.org>");
MODULE_DESCRIPTION("Ilitek ILI9806E Controller Driver");
MODULE_LICENSE("GPL");
