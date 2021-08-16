#define __STDDEF_NO_EXPORTS

#include "lv_lpng.h"
#include "zlib.h"
#include "PNGdec.h"

#define LV_PNG_USE_LV_FILESYSTEM    1


///
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "lvgl/lvgl.h"

void PNGRGB565(PNGDRAW *pDraw, uint16_t *pPixels, int iEndiannes, uint32_t u32Bkgd, int iHasAlpha);
extern "C" void lv_draw_map(const lv_area_t * map_area, const lv_area_t * clip_area,
                                              const uint8_t * map_p,
                                              const lv_draw_img_dsc_t * draw_dsc,
                                              bool chroma_key, bool alpha_byte);
                                                  
/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t decoder_info(struct _lv_img_decoder_t* decoder, const void* src, lv_img_header_t* header);
static lv_res_t decoder_open(lv_img_decoder_t* dec, lv_img_decoder_dsc_t* dsc);
static void decoder_close(lv_img_decoder_t* dec, lv_img_decoder_dsc_t* dsc);
void decoder_draw_lbl(struct _lv_img_decoder_t* decoder, struct _lv_img_decoder_dsc_t* dsc, lv_decoder_draw_context_t* lv_draw_ctx);

void lv_lpng_init(void)
{
    lv_img_decoder_t* dec = lv_img_decoder_create();
    lv_img_decoder_set_info_cb(dec, decoder_info);
    lv_img_decoder_set_open_cb(dec, decoder_open);
    lv_img_decoder_set_close_cb(dec, decoder_close);
    lv_img_decoder_set_draw_lbl_cb(dec, decoder_draw_lbl);
}

/**
 * Get info about a PNG image
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t decoder_info(struct _lv_img_decoder_t* decoder, const void* src, lv_img_header_t* header)
{
    (void)decoder; /*Unused*/
    lv_img_src_t src_type = lv_img_src_get_type(src);          /*Get the source type*/

    /*If it's a PNG file...*/
    if (src_type == LV_IMG_SRC_FILE) {
        const char* fn = (const char*)src;
        //if (!strcmp(&fn[strlen(fn) - 3], "png")) {              /*Check the extension*/
        if (strcmp(lv_fs_get_ext(fn), "png") == 0) {
            /* Read the width and height from the file. They have a constant location:
             * [16..23]: width
             * [24..27]: height
             */
            uint32_t size[2];
#if LV_PNG_USE_LV_FILESYSTEM
            lv_fs_file_t f;
            lv_fs_res_t res = lv_fs_open(&f, fn, LV_FS_MODE_RD);
            if (res != LV_FS_RES_OK) 
                return LV_RES_INV;
            lv_fs_seek(&f, 16, LV_FS_SEEK_SET);
            uint32_t rn;
            lv_fs_read(&f, &size, 8, &rn);
            if (rn != 8) 
                return LV_RES_INV;
            lv_fs_close(&f);
#else
            FILE* file;
            file = fopen(fn, "rb");
            if (!file) return LV_RES_INV;
            fseek(file, 16, SEEK_SET);
            size_t rn = fread(size, 1, 8, file);
            fclose(file);
            if (rn != 8) return LV_RES_INV;
#endif
            /*Save the data in the header*/
            header->always_zero = 0;
            header->cf = LV_IMG_CF_RAW;
            /*The width and height are stored in Big endian format so convert them to little endian*/
            header->w = (lv_coord_t)((size[0] & 0xff000000) >> 24) + ((size[0] & 0x00ff0000) >> 8);
            header->h = (lv_coord_t)((size[1] & 0xff000000) >> 24) + ((size[1] & 0x00ff0000) >> 8);

            return LV_RES_OK;
        }
    }
    /*If it's a PNG file in a  C array...*/
    else if (src_type == LV_IMG_SRC_VARIABLE) {
        return LV_RES_INV;
    }

    return LV_RES_INV;         /*If didn't succeeded earlier then it's an error*/
}


/**
 * Open a PNG image and return the decided image
 * @param src can be file name or pointer to a C array
 * @param style style of the image object (unused now but certain formats might use it)
 * @return pointer to the decoded image or  `LV_IMG_DECODER_OPEN_FAIL` if failed
 */
static lv_res_t decoder_open(lv_img_decoder_t* decoder, lv_img_decoder_dsc_t* dsc)
{
    /*If it's a PNG file...*/
    if (dsc->src_type == LV_IMG_SRC_FILE) {
        const char* fn = (const char*)dsc->src;

        //if (!strcmp(&fn[strlen(fn) - 3], "png")) {              /*Check the extension*/
        if (strcmp(lv_fs_get_ext(fn), "png") == 0) {
            dsc->img_data = NULL;
            return LV_RES_OK;     /*The image is fully decoded. Return with its pointer*/
        }
    }
    
    return LV_RES_INV;    /*If not returned earlier then it failed*/
}

/**
 * Free the allocated resources
 */
static void decoder_close(lv_img_decoder_t* decoder, lv_img_decoder_dsc_t* dsc)
{
    (void)decoder; /*Unused*/
    if (dsc->img_data) {
        free((uint8_t*)dsc->img_data);
        dsc->img_data = NULL;
    }
}

///-
PNG g_Png;
lv_fs_file_t *png_fp = NULL;
void* myOpen(const char* filename, int32_t* size) {
    //lv_fs_file_t *fp = (lv_fs_file_t*)lv_mem_alloc(sizeof(lv_fs_file_t));
    if (png_fp == NULL) {
        png_fp = (lv_fs_file_t*)lv_mem_alloc(sizeof(lv_fs_file_t));
    }
    
    lv_fs_res_t res = lv_fs_open(png_fp, filename, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK)
        return NULL;

    // get file size
    uint32_t cur = 0;
    lv_fs_tell(png_fp, &cur);
    lv_fs_seek(png_fp, 0L, LV_FS_SEEK_END);
    lv_fs_tell(png_fp, (uint32_t*)size);

    // set pos to begin
    lv_fs_seek(png_fp, 0, LV_FS_SEEK_SET);

    return png_fp;
}

void myClose(void* handle) {
    if (handle) {
        lv_fs_close((lv_fs_file_t*)handle);
        //lv_mem_free(handle);
    }
}

int32_t myRead(PNGFILE* handle, uint8_t* buffer, int32_t length) {
    if (!handle)
        return 0;
    if (handle->fHandle == NULL)
        return 0;

    uint32_t rn = 0;
    if (lv_fs_read((lv_fs_file_t*)handle->fHandle, buffer, length, &rn) != LV_FS_RES_OK)
        return 0;
    return rn;
}

int32_t mySeek(PNGFILE* handle, int32_t position) {
    if (!handle)
        return -1;
    if (handle->fHandle == NULL)
        return -2;
    if (lv_fs_seek((lv_fs_file_t*)handle->fHandle, position, LV_FS_SEEK_SET) != LV_FS_RES_OK)
        return -3;

    return 0;
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW* pDraw) {
    lv_decoder_draw_context_t* pdraw_ctx = (lv_decoder_draw_context_t*)pDraw->pUser;
    if (pdraw_ctx == NULL)
        return;

    if (pDraw->y < pdraw_ctx->y)
        return;

    lv_area_t mask_line;
    bool union_ok = _lv_area_intersect(&mask_line, pdraw_ctx->clip_area, pdraw_ctx->line);
    if (union_ok == false)
        return;

    // getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
    PNGRGB565(pDraw, (uint16_t *)pdraw_ctx->buf, PNG_RGB565_BIG_ENDIAN, 0xffffffff, pdraw_ctx->hasAlpha);
    int off = pdraw_ctx->x * 2;

    lv_draw_map(pdraw_ctx->line, &mask_line, pdraw_ctx->buf + off, (const lv_draw_img_dsc_t*)pdraw_ctx->draw_dsc, false/*chroma_keyed*/, false /*pdraw_ctx->hasAlpha*/);
    pdraw_ctx->line->y1++;
    pdraw_ctx->line->y2++;
    pdraw_ctx->y++;
    //tft.writeRect(0, pDraw->y + 24, pDraw->iWidth, 1, usPixels);
}

void decoder_draw_lbl(struct _lv_img_decoder_t* decoder, struct _lv_img_decoder_dsc_t* dsc, lv_decoder_draw_context_t* lv_draw_ctx)
{    
    decoder->user_data = &g_Png;
    int error = g_Png.open(lv_draw_ctx->img_path, myOpen, myClose, myRead, mySeek, PNGDraw);
    if (error != PNG_SUCCESS) {
        //printf("error %u: %s\n", error, lodepng_error_text(error));
        return;
    }

    //- LV_HOR_RES * 3 => 256 * 2: tile width with 565 encoded[2021/08/16]
    uint8_t* buf = (uint8_t*)lv_mem_buf_get(LV_HOR_RES * 3 /* RGB.2 A.1 */);
    if (buf == NULL) {
        g_Png.close();
        return;
    }

    lv_draw_ctx->hasAlpha = g_Png.hasAlpha();
    lv_draw_ctx->buf = buf;
    error = g_Png.decode(lv_draw_ctx, 0);
    if (error != PNG_SUCCESS) {
        //printf("error %u: %s\n", error, lodepng_error_text(error));
    }

    lv_mem_buf_release(buf);
    g_Png.close();
}

