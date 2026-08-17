file(GLOB_RECURSE C_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.[cC]"
)

idf_component_register(
    SRCS ${C_SOURCES}
    INCLUDE_DIRS
        "."
        "MyCode"
        "MyCode/App"
        "MyCode/Drv"
        "MyCode/Rtos"
        "MyCode/Sta"
        "MyCode/Config"
    PRIV_REQUIRES
        spi_flash
        esp_http_server
        esp_netif
        nvs_flash
        lwip
        esp_wifi
        esp_event
        esp_timer
)