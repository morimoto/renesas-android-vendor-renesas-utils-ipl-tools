/*
* Copyright (C) 2019 GlobalLogic

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#ifndef _IPL_H_
#define _IPL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef PACK_IPL_EMCC
#include <emmc_ipl_addr.h>
#endif
#include "openssl/sha.h"

#ifdef PACK_IPL_EMCC
#define IPL_EMMC_BOOT_MAGIC "BOOTMMC"
#define IPL_EMMC_BOOT_MAGIC_SIZE sizeof(IPL_EMMC_BOOT_MAGIC)
#else
#define IPL_BOOT_MAGIC "BOOTIPL"
#define IPL_BOOT_MAGIC_SIZE sizeof(IPL_BOOT_MAGIC)
#endif

#define IPL_FILE_NAME_SIZE 32

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))
#endif

#ifdef PACK_IPL_EMCC
#define CERT_HEADER_OFFSET  RCAR_EMMC_CERT_HEADER
#define BL31_OFFSET         RCAR_ARM_TRUSTED_ADDR
#define TEE_OFFSET          RCAR_OPTEE_ADDR
#define UBOOT_OFFSET        RCAR_UBOOT_ADDR
#define MAX_IPLS            (4)

/*IPL types*/
enum {
    CERT = 0xAA550001,
    BL31,
    OPTEE,
    UBOOT,
};
#else
#define MAX_IPLS (6)

enum {
    PARAM = 0xAA550001,
    BL2,
    CERT,
    BL31,
    OPTEE,
    UBOOT,
};
#endif

/*Digest types*/

enum {
    NO_HASH = 0xF0,
    SHA_1,
    SHA_256,
    SHA_512,
};

/*Defines IPL parameters*/
struct ipl_params {
    uint32_t ftype;     /*IPL Type: PARAM, BL2, CERT, BL31,OPTEE, UBOOT */
    uint32_t fsize;     /*Size of tthe IPL*/
    char fname[IPL_FILE_NAME_SIZE]; /*File name of the IPL. i.e. "u-boot.bin" */
    uint32_t digest_type; /*Type of the Hash function for digest*/
    uint8_t digest[64];       /*Hash of the IPL*/
    uint8_t sign[256];        /*Sign of the IPL Hash*/
} __attribute__((packed));

/*Image header structure*/
struct image_header {
#ifdef PACK_IPL_EMCC
    char ipl_magic[IPL_EMMC_BOOT_MAGIC_SIZE]; /*Bootloader image magic*/
#else
    char ipl_magic[IPL_BOOT_MAGIC_SIZE];        /*Bootloader image magic*/
#endif
    uint32_t total_size;                                                 /* Overal image size in bytes */
    uint32_t ipl_offset[MAX_IPLS];                             /*offset of specific IPL in the image*/
}__attribute__((packed));

/*Image structure*/
struct ipl_image {
    struct image_header hdr;            /*Image header*/
    struct ipl_params ipl[MAX_IPLS]; /*IPL Parameters descriptors*/
} __attribute__((packed));

struct ipls {
    uint32_t id;
    const char *file_name;
};

char *merge_path_with_fname(const char *path, const char *fname);

#endif
