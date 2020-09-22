#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "bme280.h"

struct sensor {
    uint8_t addr;
    uint8_t fd;
};

struct info {
    float temp;
    float prsr;
    float hmd;
};

int8_t stream_sensor_data(struct bme280_dev *dev);
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
void user_delay_us(uint32_t period, void *intf_ptr);

int main(int argc, char* argv[]) {
    struct bme280_dev dev;
    struct sensor id;
    int8_t result = BME280_OK;


    if (argc < 2)
    {
        fprintf(stderr, "Missing argument for i2c bus.\n");
        exit(1);
    }

    if ((id.fd = open(argv[1], O_RDWR)) < 0)
    {
        fprintf(stderr, "Failed to open the i2c bus %s\n", argv[1]);
        exit(1);
    }

    id.addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id.fd, I2C_SLAVE, id.addr) < 0)
    {
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    dev.intf_ptr = &id;

    /* Initialize the bme280 */
    result = bme280_init(&dev);
    if (result != BME280_OK)
    {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", result);
        exit(1);
    }

    result = stream_sensor_data(&dev);
    if (result != BME280_OK)
    {
        fprintf(stderr, "Failed to stream sensor data (code %+d).\n", result);
        exit(1);
    }

    return 0;
}

void user_delay_us(uint32_t period, void *intf_ptr) {
    usleep(period);
}

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    struct sensor sensor_id;

    sensor_id = *((struct sensor*) intf_ptr);

    write(sensor_id.fd, &reg_addr, 1);
    read(sensor_id.fd, data, len);

    return 0;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    uint8_t *buf;
    struct sensor id;

    id = *((struct sensor *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

int8_t stream_sensor_data(struct bme280_dev *dev) {
    FILE * file;
    int8_t result;
    uint8_t settings;
    uint32_t req_delay;
    struct info sensor_info;
    struct bme280_data comp_data;
    uint8_t i = 0;

    if((file = fopen("./data/sensor_data.csv", "a")) == NULL) {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", result);
        return -1;
    }

    if(ftell(file) == 0) {
        fprintf(file, "TEMPERATURA,PRESSAO,UMIDADE\n");
    }
    fclose(file);

    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

    settings =  BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    result = bme280_set_sensor_settings(settings, dev);

    req_delay = bme280_cal_meas_delay(&dev->settings);

    while(1) {
        file = fopen("./data/sensor_data.csv", "a+");

        result = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
        dev->delay_us(req_delay, dev->intf_ptr);
        result = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);

        if(i < 10) {
            sensor_info.temp += comp_data.temperature;
            sensor_info.prsr += comp_data.pressure;
            sensor_info.hmd += comp_data.humidity;
        }

        else {
            sensor_info.temp /= 10.0;
            sensor_info.prsr /= 10.0;
            sensor_info.hmd /= 10.0;
            fprintf(file, "%0.2f,%0.2f,%0.2f\n", sensor_info.temp, sensor_info.prsr, sensor_info.hmd);
            i = 0;
        }

        printf("%d %0.2f,%0.2f,%0.2f\n", i, comp_data.temperature, comp_data.pressure, comp_data.humidity);
        fflush(file);
        sleep(1);
        i++;
    }

    fclose(file);
    return result;
}
