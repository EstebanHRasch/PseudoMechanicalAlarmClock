deps_config := \
	/home/david/esp/esp-idf/components/app_trace/Kconfig \
	/home/david/esp/esp-idf/components/aws_iot/Kconfig \
	/home/david/esp/esp-idf/components/bt/Kconfig \
	/home/david/esp/esp-idf/components/driver/Kconfig \
	/home/david/esp/esp-idf/components/esp32/Kconfig \
	/home/david/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/david/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/david/esp/esp-idf/components/ethernet/Kconfig \
	/home/david/esp/esp-idf/components/fatfs/Kconfig \
	/home/david/esp/esp-idf/components/freertos/Kconfig \
	/home/david/esp/esp-idf/components/heap/Kconfig \
	/home/david/esp/esp-idf/components/http_server/Kconfig \
	/home/david/esp/esp-idf/components/libsodium/Kconfig \
	/home/david/esp/esp-idf/components/log/Kconfig \
	/home/david/esp/esp-idf/components/lwip/Kconfig \
	/home/david/esp/esp-idf/components/mbedtls/Kconfig \
	/home/david/esp/esp-idf/components/mdns/Kconfig \
	/home/david/esp/esp-idf/components/openssl/Kconfig \
	/home/david/esp/esp-idf/components/pthread/Kconfig \
	/home/david/esp/esp-idf/components/spi_flash/Kconfig \
	/home/david/esp/esp-idf/components/spiffs/Kconfig \
	/home/david/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/david/esp/esp-idf/components/vfs/Kconfig \
	/home/david/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/david/esp/esp-idf/Kconfig.compiler \
	/home/david/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/david/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/david/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/david/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
