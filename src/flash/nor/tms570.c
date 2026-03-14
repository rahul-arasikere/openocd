/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2026 Rahul Vagish Arasikere                             *
 *   arasikere |_dot_| rahul |_at_| gmail |_dot_| com                      *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "helper/bitfield.h"
#include "helper/bits.h"
#include "imp.h"
/** Target Address Helpers
 */

const target_addr_t DEV_ID = 0xFFFFFFF0;
const target_addr_t FBPROT = 0xFFF87030;
const target_addr_t FBSE = 0xFFF87034;
const target_addr_t FMAC = 0xFFF87050;
const target_addr_t FSMWRENA = 0xFFF87288;

const uint32_t EEPROM_BANK = 7;
/* ----------------------------------------------------------------------
 */

static const struct flash_sector rm57l843_bank0_sectors[] = {
	{0x00000000, 0x00004000, -1, -1}, {0x00005000, 0x00004000, -1, -1},
	{0x00008000, 0x00004000, -1, -1}, {0x0000C000, 0x00004000, -1, -1},
	{0x00010000, 0x00004000, -1, -1}, {0x00014000, 0x00004000, -1, -1},
	{0x00018000, 0x00008000, -1, -1}, {0x00020000, 0x00020000, -1, -1},
	{0x00040000, 0x00020000, -1, -1}, {0x00060000, 0x00020000, -1, -1},
	{0x00080000, 0x00040000, -1, -1}, {0x000C0000, 0x00040000, -1, -1},
	{0x00100000, 0x00040000, -1, -1}, {0x00140000, 0x00040000, -1, -1},
	{0x00180000, 0x00040000, -1, -1}, {0x001C0000, 0x00040000, -1, -1},
};

#define RM57L843_BANK0_NUM_SECTORS ARRAY_SIZE(rm57l843_bank0_sectors)

static const struct flash_sector rm57l843_bank1_sectors[] = {
	{0x00200000, 0x00020000, -1, -1}, {0x00220000, 0x00020000, -1, -1},
	{0x00240000, 0x00020000, -1, -1}, {0x00260000, 0x00020000, -1, -1},
	{0x00280000, 0x00020000, -1, -1}, {0x002A0000, 0x00020000, -1, -1},
	{0x002C0000, 0x00020000, -1, -1}, {0x002E0000, 0x00020000, -1, -1},
	{0x00300000, 0x00020000, -1, -1}, {0x00320000, 0x00020000, -1, -1},
	{0x00340000, 0x00020000, -1, -1}, {0x00360000, 0x00020000, -1, -1},
	{0x00380000, 0x00020000, -1, -1}, {0x003A0000, 0x00020000, -1, -1},
	{0x003C0000, 0x00020000, -1, -1}, {0x003E0000, 0x00020000, -1, -1},
};

#define RM57L843_BANK1_NUM_SECTORS ARRAY_SIZE(rm57l843_bank1_sectors)

static const struct flash_sector rm57l843_bank7_sectors[] = {
	{0xF0200000, 0x00001000, -1, -1}, {0xF0201000, 0x00001000, -1, -1},
	{0xF0202000, 0x00001000, -1, -1}, {0xF0203000, 0x00001000, -1, -1},
	{0xF0204000, 0x00001000, -1, -1}, {0xF0205000, 0x00001000, -1, -1},
	{0xF0206000, 0x00001000, -1, -1}, {0xF0207000, 0x00001000, -1, -1},
	{0xF0208000, 0x00001000, -1, -1}, {0xF0209000, 0x00001000, -1, -1},
	{0xF020A000, 0x00001000, -1, -1}, {0xF020B000, 0x00001000, -1, -1},
	{0xF020C000, 0x00001000, -1, -1}, {0xF020D000, 0x00001000, -1, -1},
	{0xF020E000, 0x00001000, -1, -1}, {0xF020F000, 0x00001000, -1, -1},
	{0xF0210000, 0x00001000, -1, -1}, {0xF0211000, 0x00001000, -1, -1},
	{0xF0212000, 0x00001000, -1, -1}, {0xF0213000, 0x00001000, -1, -1},
	{0xF0214000, 0x00001000, -1, -1}, {0xF0215000, 0x00001000, -1, -1},
	{0xF0216000, 0x00001000, -1, -1}, {0xF0217000, 0x00001000, -1, -1},
	{0xF0218000, 0x00001000, -1, -1}, {0xF0219000, 0x00001000, -1, -1},
	{0xF021A000, 0x00001000, -1, -1}, {0xF021B000, 0x00001000, -1, -1},
	{0xF021C000, 0x00001000, -1, -1}, {0xF021D000, 0x00001000, -1, -1},
	{0xF021E000, 0x00001000, -1, -1}, {0xF021F000, 0x00001000, -1, -1},
};

#define RM57L843_BANK7_NUM_SECTORS ARRAY_SIZE(rm57l843_bank7_sectors)

/** Driver Private Data Sttructure
 */
struct tms570_flash_driver {
	bool probed;
	const char *name;
	uint32_t flash_bank;
	uint32_t device_id;
};

/** Commands
 */

static const struct command_registration tms570_flash_commands[] = {
	COMMAND_REGISTRATION_DONE,
};

/** probe command
 */

static int tms570_probe(struct flash_bank *bank)
{
	uint32_t dev_id = 0;
	struct tms570_flash_driver *priv = bank->driver_priv;
	target_read_u32(bank->target, DEV_ID, &dev_id);
	uint32_t platform_id = FIELD_GET(0x7, dev_id);
	uint32_t unique_id = FIELD_GET(0x3FFF << 17, dev_id);
	if (platform_id != 0x5) {
		LOG_ERROR("tms570: unknown platform id: 0x%x, expected: 0x5!", platform_id);
		return ERROR_FLASH_OPERATION_FAILED;
	}

	free(bank->sectors);
	bank->sectors = NULL;
	bank->num_sectors = 0;

	switch (unique_id) {
		case 0x0022 /* RM57L843 */:
			priv->name = "RM57L843";
			if (bank->base < 0x0020000) {
				bank->sectors = malloc(sizeof(rm57l843_bank0_sectors));
				if (!bank->sectors)
					return -ENOMEM;
				memcpy(bank->sectors, rm57l843_bank0_sectors,
					sizeof(rm57l843_bank0_sectors));
				bank->size = 0x200000;
				bank->num_sectors = RM57L843_BANK0_NUM_SECTORS;
				priv->flash_bank = 0;
			} else if (bank->base >= 0x00200000 && bank->base < 0x00400000) {
				bank->sectors = malloc(sizeof(rm57l843_bank1_sectors));
				if (!bank->sectors)
					return -ENOMEM;
				memcpy(bank->sectors, rm57l843_bank1_sectors,
					sizeof(rm57l843_bank1_sectors));
				bank->size = 0x200000;
				bank->num_sectors = RM57L843_BANK1_NUM_SECTORS;
				priv->flash_bank = 1;
			} else if (bank->base == 0xF0200000 && bank->base < 0xF0220000) {
				bank->sectors = malloc(sizeof(rm57l843_bank7_sectors));
				if (!bank->sectors)
					return -ENOMEM;
				memcpy(bank->sectors, rm57l843_bank7_sectors,
					sizeof(rm57l843_bank7_sectors));
				bank->size = 0x20000;
				bank->num_sectors = RM57L843_BANK7_NUM_SECTORS;
				priv->flash_bank = 7;
			} else
				return ERROR_FLASH_BANK_INVALID;
			priv->probed = 1;
			break;

		default:
			LOG_ERROR("tms570: unsupported part number: %x", unique_id);
			return ERROR_FLASH_OPERATION_FAILED;
	}
	return ERROR_OK;
}

/** info command
 */

static int tms570_info(struct flash_bank *bank,
	struct command_invocation *cmd)
{
	struct tms570_flash_driver *priv = bank->driver_priv;
	if (!priv->probed)
		tms570_probe(bank);

	if (!priv->probed) {
		command_print(
			cmd, "Couldn't identify the target 0x%x as part of the TMS570 family!",
			priv->device_id);
		return ERROR_FLASH_OPERATION_FAILED;
	}
	command_print(cmd, "Identified part: %s", priv->name);
	return ERROR_OK;
}

/**
 * Check protection on bank/sectors
 */

static int tms570_protect_check(struct flash_bank *bank)
{
	uint32_t fmac = 0;
	uint32_t fbse = 0;
	struct target *target = bank->target;
	struct tms570_flash_driver *priv = bank->driver_priv;
	if (target->state != TARGET_HALTED) {
		LOG_ERROR("tms570: target not halted!");
		return ERROR_TARGET_NOT_HALTED;
	}

	if (!priv->probed)
		tms570_probe(bank);
    
	/*unlock fsm for operations*/
	target_write_u32(target, FSMWRENA, 0x5);
	target_read_u32(target, FMAC, &fmac);
	target_write_u32(target, FMAC, priv->flash_bank);
	target_read_u32(target, FMAC, &fmac);
	if (FIELD_GET(0x7, fmac) != priv->flash_bank) {
		LOG_ERROR("tms570: fmac: 0x%x, could not select bank: %u", fmac, priv->flash_bank);
		return ERROR_FLASH_BANK_INVALID;
	}
	if (priv->flash_bank != EEPROM_BANK) {
		target_read_u32(target, FBSE, &fbse);
		for (unsigned int sector = 0; sector < bank->num_sectors; sector++)
			bank->sectors[sector].is_protected = (fbse & BIT(sector))? 0 : 1;
	} else {
		for (unsigned int sector = 0; sector < bank->num_sectors; sector++)
			bank->sectors[sector].is_protected = 0;
	}
	return ERROR_OK;
}

/** flash bank tms570 <base> <size> <chip_width> <bus_width> <target>
 * [options...]
 */

FLASH_BANK_COMMAND_HANDLER(tms570_flash_command_handler) {
	if (!bank->driver_priv)
		bank->driver_priv = malloc(sizeof(struct tms570_flash_driver));

	if (!bank->driver_priv)
		return -ENOMEM;

	(void)memset(bank->driver_priv, 0, sizeof(struct tms570_flash_driver));
	return ERROR_OK;
}

const struct flash_driver tms570_flash = {
	.name = "tms570",
	.commands = tms570_flash_commands,
	.flash_bank_command = tms570_flash_command_handler,
	.info = tms570_info,
	.probe = tms570_probe,
	.auto_probe = tms570_probe,
	.free_driver_priv = default_flash_free_driver_priv,
	.protect_check = tms570_protect_check,
};
