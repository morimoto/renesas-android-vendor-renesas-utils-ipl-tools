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


#include "ipl.h"

#define MAKE_ALL "all"
#define IMG_TYPE "-t"
#define IMG_FILE "-f"


/*Set image id and filename*/
#ifdef PACK_IPL_EMCC
static const struct ipls ipl_file[MAX_IPLS] = {
        {.id = CERT, .file_name = "cert_header_sa6.bin"},
        {.id = BL31, .file_name = "bl31.bin"},
        {.id = OPTEE, .file_name = "tee.bin"},
        {.id = UBOOT, .file_name = "u-boot.bin"},
};
#else
static const struct ipls ipl_file[MAX_IPLS] = {
        {.id = PARAM, .file_name = "bootparam_sa0_hf.bin"},
        {.id = BL2, .file_name = "bl2_hf.bin"},
        {.id = CERT, .file_name = "cert_header_sa6_hf.bin"},
        {.id = BL31, .file_name = "bl31_hf.bin"},
        {.id = OPTEE, .file_name = "tee_hf.bin"},
        {.id = UBOOT, .file_name = "u-boot_hf.bin"},
};

static const struct ipls unpacked_files[MAX_IPLS] = {
        {.id = PARAM, .file_name = "bootparam_sa0.bin"},
        {.id = BL2, .file_name = "bl2.bin"},
        {.id = CERT, .file_name = "cert_header_sa6.bin"},
        {.id = BL31, .file_name = "bl31.bin"},
        {.id = OPTEE, .file_name = "tee.bin"},
        {.id = UBOOT, .file_name = "u-boot.bin"},
};
#endif

static struct ipl_image  img;

#ifdef PACK_IPL_EMCC
#define IPL_IMAGE "bootloader.img"

static uint32_t offsets[MAX_IPLS] = {
    CERT_HEADER_OFFSET, BL31_OFFSET,
    TEE_OFFSET, UBOOT_OFFSET,
};

/*
 * Check if ipl binary fits to window between bootloaders.
 * Last ipl is not being checked, because u-boot can change total
 * bootloader size dynamically.
 */
static bool check_ipl_size(int ipl_idx, uint32_t fsize)
{
    if (ipl_idx == MAX_IPLS - 1)
        return true;

    return ((offsets[ipl_idx + 1] - offsets[ipl_idx]) >= fsize);
}

static uint32_t get_ipl_offset(int ipl_idx)
{
    return offsets[ipl_idx] - sizeof(struct ipl_params);
}

#else
#define IPL_IMAGE "bootloader_hf.img"
#endif

static void free_bufs(uint8_t *buf[MAX_IPLS], int n)
{
    int i;
    for (i = 0;i < n; i++)
        free(buf[i]);
}

static int pack_bootloader(const char *ipl, const char *path)
{
    FILE *fimg;
    FILE *fipl;
    char *fipl_name= NULL;
    int i, ret;
    uint32_t len;
    struct stat st;
    uint8_t *buf[MAX_IPLS];

    if (strncmp(ipl,MAKE_ALL, sizeof(MAKE_ALL))) {
        printf("Error: All bootloadershas to be specified\n");
        return -1;
    }

    /*Set total image size to offset of the struct ipl_params.
    *   Supposed to be 36;
    */
    img.hdr.total_size = sizeof(struct image_header);
    printf("ipl_image.hdr size = %d\n",  img.hdr.total_size);
    /*Set image header*/
#ifdef PACK_IPL_EMCC
	memcpy (img.hdr.ipl_magic, IPL_EMMC_BOOT_MAGIC, sizeof(img.hdr.ipl_magic));
#else
    memcpy (img.hdr.ipl_magic, IPL_BOOT_MAGIC, sizeof(img.hdr.ipl_magic));
#endif

    for (i = 0; i< MAX_IPLS; i++) {
        buf[i] = NULL;
        /*Concat path with filename*/
        fipl_name = merge_path_with_fname(path,ipl_file[i].file_name);
        ret = stat(fipl_name, &st);
        if (ret) {
            printf("WARNING: Can't find IPL (%s)\n", fipl_name);
            free(fipl_name);
            continue;
        }

        fipl = fopen(fipl_name, "rb");
        free(fipl_name);
        if (fipl == NULL) {
                printf("fopen error (%s/%s)\n", path, ipl_file[i].file_name);
                break;
        }

        /*Set ipl parameters*/
        img.ipl[i].ftype = ipl_file[i].id;
        img.ipl[i].fsize = st.st_size;
        len = strlen(ipl_file[i].file_name);
        if (len >= sizeof(img.ipl[i].fname)) {
            printf("WARNING: File name is too long (%s), will be truncated\n",
                ipl_file[i].file_name);
            len =  sizeof(img.ipl[i].fname) -1;
        }
#ifdef PACK_IPL_EMCC
        memcpy (img.ipl[i].fname, ipl_file[i].file_name, len);
#else
        memcpy (img.ipl[i].fname, unpacked_files[i].file_name, len);
#endif
        img.ipl[i].digest_type = SHA_256;
        /*Allocate memory for binary file*/
        buf[i] = (uint8_t *) malloc(img.ipl[i].fsize);
        if (!buf[i]) {
            printf("Error: Memory allocation failed for IPL %s\n",
                img.ipl[i].fname);
            fclose(fipl);
            break;
        }
        memset(buf[i], 0, img.ipl[i].fsize);
        len = fread(buf[i], 1U, st.st_size, fipl);
        fclose(fipl);

#ifdef PACK_IPL_EMCC
        if (!check_ipl_size(i, img.ipl[i].fsize)) {
            printf("ERROR: not enough size in image for %s\n", img.ipl[i].fname);
            break;
        }
#endif

        if (len != st.st_size) {
            printf("File read Error. IPL: %s\n", img.ipl[i].fname);
            break;
        }

        /*unsigned char *SHA256(const unsigned char *d, size_t n,
      unsigned char *md);*/
        SHA256(buf[i], img.ipl[i].fsize, img.ipl[i].digest);

        /*TODO: we have to add sign*/
#ifdef PACK_IPL_EMCC
        img.hdr.ipl_offset[i] = get_ipl_offset(i);
#else
        img.hdr.ipl_offset[i] = img.hdr.total_size;
        img.hdr.total_size += sizeof(struct ipl_params) + img.ipl[i].fsize;
#endif
    }

    if (i != MAX_IPLS) {
        printf("Exiting due to previous errors\n");
        free_bufs(buf, i);
        return -1;
    }
#ifdef PACK_IPL_EMCC
    img.hdr.total_size = get_ipl_offset(i - 1) + sizeof(struct ipl_params)
          + img.ipl[i - 1].fsize;
#endif

    /*Start writing */

    fipl_name = merge_path_with_fname(path, IPL_IMAGE);
    fimg = fopen(fipl_name, "wb");
    if (fimg == NULL) {
            printf("fopen error (%s)\n", IPL_IMAGE);
            free_bufs(buf, MAX_IPLS);
            free(fipl_name);
            return -1;
    }
     /*Write image header*/
    len = fwrite(&img.hdr, sizeof(struct image_header ), 1, fimg);
    if (len != 1) {
        printf("File write error %s\n", IPL_IMAGE);
        fclose(fimg);
        free_bufs(buf, MAX_IPLS);
        free(fipl_name);
        return -1;
    }

    /*Write IPL parameters and data buffers*/
    for (i = 0; i< MAX_IPLS; i++) {
            /*Check if we have the appropriate IPL*/
            if (img.ipl[i].fsize) {
#ifdef PACK_IPL_EMCC
                fseek(fimg, get_ipl_offset(i), SEEK_SET);
#endif
                len = fwrite(&img.ipl[i], sizeof(struct ipl_params), 1, fimg);
                if (len != 1) {
                    printf("File write error %s\n", IPL_IMAGE);
                    break;
                }

                len = fwrite(buf[i], img.ipl[i].fsize, 1, fimg);
                if (len != 1) {
                    printf("File write error %s\n", IPL_IMAGE);
                    break;
                }
            }
    }
    fclose(fimg);

    /*Check file consistency*/
    ret = stat(fipl_name, &st);
    if ((i != MAX_IPLS) || (st.st_size != img.hdr.total_size)) {
        printf ("Created image for %d bootloaders\n", i);
        printf ("Header file size = %d, real file size = %ld\n", img.hdr.total_size,
            st.st_size);
        printf("Error: Image can not be used for IPL flashing!Destroying.\n");
        ret = remove(fipl_name);
        if (ret)
            printf("Can't remove %s file!\n",     fipl_name);
        ret = -1;
    } else
    {
        printf("Image created successfully\n");
        ret = 0;
    }
    free_bufs(buf, MAX_IPLS);
    free(fipl_name);
    return ret;
}

static void usage(void)
{
    printf("\r\n");
    printf("        pack_ipl all [<path>]\n");
    printf("            if <path> not set, it's trated as <./>\n");
    printf("\r\n");
}


/*
*makeipl all <./>
*makeipl -t uboot -f ./uboot.bin
* FIXME: Currently only <makeipl all> option supported
*/
int main(int argc, char **argv)
{
    int ret;
    char default_path[] = "./";
    if (argc == 1) {
        usage();
        return -1;
    }

    if (!strncmp(argv[1],MAKE_ALL, sizeof(MAKE_ALL))) {
        char *path;
        if (argc == 3)
            path = argv[2];
        else
            path = default_path;
        ret  = pack_bootloader(MAKE_ALL, path);
    } else {
        usage();
        return -1;
    }

}

