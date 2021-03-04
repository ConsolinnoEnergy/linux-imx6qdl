
static const struct ethtool_ops dm9051_ethtool_ops = {
	.get_drvinfo		= dm9051_get_drvinfo,
#if LNX_KERNEL_v58
	.get_link_ksettings	= dm9000_get_link_ksettings,
	.set_link_ksettings	= dm9000_set_link_ksettings,
#else
	.get_settings		= dm9000_get_settings,
	.set_settings		= dm9000_set_settings,
#endif
	.get_msglevel		= dm9000_get_msglevel,
	.set_msglevel		= dm9000_set_msglevel,
	.nway_reset			= dm9000_nway_reset,
	.get_link			= dm9000_get_link,
/*TBD	
	.get_wol			= dm9000_get_wol,
	.set_wol			= dm9000_set_wol,
*/
 	.get_eeprom_len		= dm9000_get_eeprom_len,
 	.get_eeprom			= dm9000_get_eeprom,
 	.set_eeprom			= dm9000_set_eeprom,
};        
