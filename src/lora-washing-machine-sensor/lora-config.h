#include "lora-ids.h"

/* The sync word functions like a "tag": This device will onl listen to
 * messages with this sync word (tag) and completely ignore all others */
#define LORA_SYNC_WORD 0x42 // 66

/* If desired change this devices ID.
 * If so, also change the LORA_DEVICE_IDS map in "lora-ids.h" */
#define LORA_DEVICE_ID_A 0xB // 11
#define LORA_DEVICE_ID_B 0xC // 12

#define LORA_GATEWAY_ID 0x1 // 1
