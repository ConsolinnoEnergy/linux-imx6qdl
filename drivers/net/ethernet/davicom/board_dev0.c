													     
#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information! */
#else
static struct spi_board_info dm9051_spi_board_devs[] __initdata = {
	[0] = {
	.modalias = "dm9051",
	.max_speed_hz = DRV_MAX_SPEED_HZ,
	.bus_num = DRV_SPI_BUS_NUMBER,
	.chip_select = DRV_SPI_CHIP_SELECT,
	.mode = SPI_MODE_0,
  	  #ifdef DM_CONF_POLLALL_INTFLAG 
  	  // or, while swap between polling & interrupt and also conf module, .irq should keep same value for doing "dm9051_device_spi_delete()." 
	 .irq = DM_CONF_INTERRUPT_IRQ,  
  	  #endif
	},
};
#endif
