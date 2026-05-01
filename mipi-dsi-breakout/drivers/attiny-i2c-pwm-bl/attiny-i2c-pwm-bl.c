// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for an I2C device to generate a PWM signal for use in a MIPI DSI
 * screen backlight.
 *
 * Copyright (C) 2026 Cory Parsnipson <cparsnipson@gmail.com>
 *
 * based on the pwm-pca9685.c, rpi_backlight.c, and pwm_bl.c drivers
 */

#include <linux/backlight.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/types.h>

#define BACKLIGHT_DEVICE_ID "tst043wvbi"        // determines /sys/class/backlight subfolder
#define DEVICE_ID           "attiny_i2c_pwm_bl"
#define DEVICE_ID_HYPHENS   "attiny-i2c-pwm-bl" // determines driver name

#define REG_ADDR_PWM_DUTY_LOW       3
#define REG_ADDR_PWM_DUTY_HIGH      4

struct attiny_i2c_pwm_device {
	struct regmap *regmap;
	struct i2c_client *client;
};

static const struct regmap_config regmap_i2c_config = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = REG_ADDR_PWM_DUTY_HIGH,
	.cache_type = REGCACHE_FLAT,
};


static int attiny_backlight_update_status(struct backlight_device *bd)
{
	struct attiny_i2c_pwm_device *device = bl_get_data(bd);
	u16 brightness;
	int err;
	
	brightness = backlight_get_brightness(bd);

	err = regmap_write(device->regmap, REG_ADDR_PWM_DUTY_LOW, brightness & 0xFF);
	if (err) {
		dev_err(&device->client->dev, "Register write to register 0x%x failed: %pe\n", REG_ADDR_PWM_DUTY_LOW, ERR_PTR(err));
		return err;
	}

	err = regmap_write(device->regmap, REG_ADDR_PWM_DUTY_HIGH, (brightness >> 8) & 0xFF);
	if (err) {
		dev_err(&device->client->dev, "Register write to register 0x%x failed: %pe\n", REG_ADDR_PWM_DUTY_HIGH, ERR_PTR(err));
		return err;
	}

	return 0;
}

static const struct backlight_ops attiny_backlight_ops = {
	.update_status = attiny_backlight_update_status,
};

static int attiny_i2c_pwm_driver_probe(struct i2c_client *client)
{
	struct backlight_properties bl_props = {
		.type           = BACKLIGHT_RAW,
		.max_brightness = 65535,
		.brightness     = 32768,
	};
	struct backlight_device *bd;
	struct attiny_i2c_pwm_device *device;

	device = devm_kzalloc(&client->dev, sizeof(*device), GFP_KERNEL);
	if (!device)
		return -ENOMEM;

	device->client = client;
	device->regmap = devm_regmap_init_i2c(client, &regmap_i2c_config);
	if (IS_ERR(device->regmap)) {
		int ret = PTR_ERR(device->regmap);
		dev_err(&client->dev, "Failed to initialize register map: %d\n", ret);
		return ret;
	}
	i2c_set_clientdata(client, device);

	bd = devm_backlight_device_register(
		&client->dev,
		BACKLIGHT_DEVICE_ID,
		&client->dev,
		device,
		&attiny_backlight_ops,
		&bl_props
	);

	if (IS_ERR(bd)) {
		dev_err(&client->dev, "Failed to register backlight\n");
		return PTR_ERR(bd);
	}
	return 0;
}

static const struct of_device_id attiny_i2c_pwm_driver_of_match[] = {
	{ .compatible = "slackerparadise," DEVICE_ID_HYPHENS },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, attiny_i2c_pwm_driver_of_match);

static const struct i2c_device_id attiny_i2c_pwm_driver_id[] = {
	{ DEVICE_ID_HYPHENS, 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, attiny_i2c_pwm_driver_id);

static struct i2c_driver attiny_i2c_pwm_driver = {
	.driver = {
		.name = DEVICE_ID,
		.of_match_table = attiny_i2c_pwm_driver_of_match,
	},
	.probe = attiny_i2c_pwm_driver_probe,
	.id_table = attiny_i2c_pwm_driver_id,
};
module_i2c_driver(attiny_i2c_pwm_driver);

MODULE_AUTHOR("Cory Parsnipson <cparsnipson@gmail.com>");
MODULE_DESCRIPTION("I2C enabled PWM LCD backlight driver");
MODULE_LICENSE("GPL");
