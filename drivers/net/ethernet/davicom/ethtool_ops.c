
/* ethtool ops block (to be "dm9051_ethtool.c") */
/*[DM9051_Ethtool_Ops.c]*/
static inline board_info_t *to_dm9051_board(struct net_device *dev)
{
	return netdev_priv(dev);
}

static void dm9051_get_drvinfo(struct net_device *dev,
				struct ethtool_drvinfo *info)
{
	strcpy(info->driver, CARDNAME_9051);
	strcpy(info->version, DRV_VERSION);
	strlcpy(info->bus_info, dev_name(dev->dev.parent), sizeof(info->bus_info));
}

static void dm9000_set_msglevel(struct net_device *dev, u32 value)
{
	board_info_t *dm = to_dm9051_board(dev);
	dm->msg_enable = value;
}

static u32 dm9000_get_msglevel(struct net_device *dev)
{
	board_info_t *dm = to_dm9051_board(dev);
	return dm->msg_enable;
}

#if LNX_KERNEL_v58
static int dm9000_get_link_ksettings(struct net_device *dev,
				     struct ethtool_link_ksettings *cmd)
{
	struct board_info *dm = to_dm9051_board(dev);
	mii_ethtool_get_link_ksettings(&dm->mii, cmd);
	return 0;
}
static int dm9000_set_link_ksettings(struct net_device *dev,
				     const struct ethtool_link_ksettings *cmd)
{
	struct board_info *dm = to_dm9051_board(dev);
	return mii_ethtool_set_link_ksettings(&dm->mii, cmd);
}

#else
static int dm9000_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	board_info_t *dm = to_dm9051_board(dev);
	mii_ethtool_gset(&dm->mii, cmd);
	return 0;
}

static int dm9000_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	board_info_t *dm = to_dm9051_board(dev);
	return mii_ethtool_sset(&dm->mii, cmd);
}
#endif

static int dm9000_nway_reset(struct net_device *dev)
{
	board_info_t *dm = to_dm9051_board(dev);
	return mii_nway_restart(&dm->mii);
}

static u32 dm9000_get_link(struct net_device *dev)
{
	board_info_t *dm = to_dm9051_board(dev);
	return (u32)dm->linkBool;
}

#define DM_EEPROM_MAGIC		(0x444D394B)

static int dm9000_get_eeprom_len(struct net_device *dev)
{
	return 128;
}

static int dm9000_get_eeprom(struct net_device *dev,
			     struct ethtool_eeprom *ee, u8 *data)
{

	printk("[dm9051_get_eeprom]\n");

#if 0
	board_info_t *dm = to_dm9051_board(dev);
	int offset = ee->offset;
	int len = ee->len;
	int i;

	// EEPROM access is aligned to two bytes 
	if ((len & 1) != 0 || (offset & 1) != 0)
		return -EINVAL;

	ee->magic = DM_EEPROM_MAGIC;

	for (i = 0; i < len; i += 2)
		dm9051_read_eeprom(dm, (offset + i) / 2, data + i);
#endif
	return 0;
}

static int dm9000_set_eeprom(struct net_device *dev,
			     struct ethtool_eeprom *ee, u8 *data)
{	
	printk("[dm9051_set_eeprom]\n");

#if 0	
	board_info_t *dm = to_dm9051_board(dev);
	int offset = ee->offset;
	int len = ee->len;
	int i;

	// EEPROM access is aligned to two bytes 
	if ((len & 1) != 0 || (offset & 1) != 0)
		return -EINVAL;

	if (ee->magic != DM_EEPROM_MAGIC)
		return -EINVAL;

	for (i = 0; i < len; i += 2)
		dm9051_write_eeprom(dm, (offset + i) / 2, data + i);
#endif		
	return 0;
}
