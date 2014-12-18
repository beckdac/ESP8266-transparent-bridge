#ifndef __FLASH_PARAM_H__
#define __FLASH_PARAM_H__

#define FLASH_PARAM_MAGIC	8255
#define FLASH_PARAM_VERSION	3

typedef struct flash_param {
	uint32_t magic;
	uint32_t version;
	uint32_t baud;
	uint32_t port;
} flash_param_t;

flash_param_t *flash_param_get(void);

#endif /* __FLASH_PARAM_H__ */
