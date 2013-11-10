/*
 * libhpcalcs: hand-helds support libraries.
 * Copyright (C) 2013 Lionel Debroux
 * Code patterns and snippets borrowed from libticables & libticalcs:
 * Copyright (C) 1999-2009 Romain Li�vin
 * Copyright (C) 2009-2013 Lionel Debroux
 * Copyright (C) 1999-2013 libti* contributors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * \file prime_cmd.c Calcs: Prime commands
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <hpcalcs.h>
#include "prime_cmd.h"
#include "logging.h"
#include "error.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static inline uint16_t crc16_block(const uint8_t * buffer, uint32_t len) {
    static const uint16_t ccitt_crc16_table[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
    };
    uint16_t crc = 0;

    while (len--) {
       crc = ccitt_crc16_table[(crc >> 8) ^ *buffer++] ^ (crc << 8);
    }
    return crc;
}

static int read_vtl_pkt(calc_handle * handle, uint8_t cmd, prime_vtl_pkt ** pkt, int packet_contains_header) {
    int res;
    (void)packet_contains_header;
    *pkt = prime_vtl_pkt_new(0);
    if (*pkt != NULL) {
        (*pkt)->cmd = cmd;
        res = prime_recv_data(handle, *pkt);
        if (res == ERR_SUCCESS) {
            if ((*pkt)->size > 0) {
                if ((*pkt)->data[0] == (*pkt)->cmd) {
                    hpcalcs_debug("%s: command matches returned data", __FUNCTION__);
                }
                else {
                    hpcalcs_debug("%s: command does not match returned data", __FUNCTION__);
                    // It's not necessarily an error.
                }
            }
            else {
                hpcalcs_info("%s: empty packet", __FUNCTION__);
            }
        }
        else {
            prime_vtl_pkt_del(*pkt);
            *pkt = NULL;
        }
    }
    else {
        res = ERR_MALLOC;
        hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
    }
    return res;
}

// Plain C equivalent of char_traits<char16_t>::length.
static uint32_t char16_strlen(char16_t * str) {
    uint32_t i = 0;
    while (*str) {
        i++;
        str++;
    }
    return i;
}

HPEXPORT int HPCALL calc_prime_s_check_ready(calc_handle * handle) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt = prime_vtl_pkt_new(2);
        if (pkt != NULL) {
            uint8_t * ptr;

            pkt->cmd = CMD_PRIME_CHECK_READY;
            ptr = pkt->data;
//            *ptr++ = 0x00; // Report number
//            *ptr++ = 0x00; // ?
            *ptr++ = CMD_PRIME_CHECK_READY;
            res = prime_send_data(handle, pkt);
            prime_vtl_pkt_del(pkt);
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_r_check_ready(calc_handle * handle, uint8_t ** out_data, uint32_t * out_size) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt;
        res = read_vtl_pkt(handle, CMD_PRIME_CHECK_READY, &pkt, 0);
        if (res == ERR_SUCCESS && pkt != NULL) {
            if (out_data != NULL && out_size != NULL) {
                *out_size = pkt->size;
                *out_data = malloc(pkt->size);
                if (*out_data != NULL) {
                    memcpy(*out_data, pkt->data, pkt->size);
                    res = 0;
                }
                else {
                    res = ERR_MALLOC;
                    hpcalcs_error("%s: couldn't allocate memory", __FUNCTION__);
                }
            }
            // else do nothing. res is already 0.
            prime_vtl_pkt_del(pkt);
        }
        else {
            hpcalcs_error("%s: failed to read packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_s_get_infos (calc_handle * handle) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt = prime_vtl_pkt_new(2);
        if (pkt != NULL) {
            uint8_t * ptr;

            pkt->cmd = CMD_PRIME_GET_INFOS;
            ptr = pkt->data;
//            *ptr++ = 0x00; // Report number
//            *ptr++ = 0x00; // ?
            *ptr++ = CMD_PRIME_GET_INFOS;
            res = prime_send_data(handle, pkt);
            prime_vtl_pkt_del(pkt);
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_r_get_infos (calc_handle * handle, calc_infos * infos) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt;
        res = read_vtl_pkt(handle, CMD_PRIME_GET_INFOS, &pkt, 1);
        if (res == 0 && pkt != NULL) {
            if (infos != NULL) {
                infos->size = pkt->size;
                infos->data = malloc(pkt->size);
                if (infos->data != NULL) {
                    memcpy(infos->data, pkt->data, pkt->size);
                    res = ERR_SUCCESS;
                }
                else {
                    res = ERR_MALLOC;
                    hpcalcs_error("%s: couldn't allocate memory", __FUNCTION__);
                }
            }
            // else do nothing. res is already 0.
            prime_vtl_pkt_del(pkt);
        }
        else {
            hpcalcs_error("%s: failed to read packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_s_recv_screen(calc_handle * handle, calc_screenshot_format format) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt = prime_vtl_pkt_new(3);
        if (pkt != NULL) {
            uint8_t * ptr;

            pkt->cmd = CMD_PRIME_RECV_SCREEN;
            ptr = pkt->data;
//            *ptr++ = 0x00; // Report number
//            *ptr++ = 0x00; // ?
            *ptr++ = CMD_PRIME_RECV_SCREEN;
            *ptr++ = (uint8_t)format;
            res = prime_send_data(handle, pkt);
            prime_vtl_pkt_del(pkt);
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_r_recv_screen(calc_handle * handle, calc_screenshot_format format, uint8_t ** out_data, uint32_t * out_size) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt;
        res = read_vtl_pkt(handle, CMD_PRIME_RECV_SCREEN, &pkt, 1);
        if (res == 0 && pkt != NULL) {
            if (pkt->size > 13) {
                // Packet has CRC
                uint16_t computed_crc; // 0x0000 ?
                uint8_t * ptr = pkt->data;
                uint16_t embedded_crc = (((uint16_t)(ptr[7])) << 8) | ((uint16_t)(ptr[6]));
                // Reset CRC before computing
                ptr[6] = 0x00;
                ptr[7] = 0x00;
                computed_crc = crc16_block(ptr, pkt->size - 6); // The CRC contains the initial 0x00, but not the final 6 bytes (...).
                hpcalcs_info("%s: embedded=%" PRIX16 " computed=%" PRIX16, __FUNCTION__, embedded_crc, computed_crc);
                if (computed_crc != embedded_crc) {
                    hpcalcs_error("%s: CRC mismatch", __FUNCTION__);
                    // TODO: change res.
                }

                // Skip marker.
                if (pkt->data[8] == (uint8_t)format && pkt->data[9] == 0xFF && pkt->data[10] == 0xFF && pkt->data[11] == 0xFF && pkt->data[12] == 0xFF) {
                    if (out_data != NULL && out_size != NULL) {
                        uint32_t size = pkt->size - 13;
                        *out_size = size;
                        *out_data = malloc(size);
                        if (*out_data != NULL) {
                            memcpy(*out_data, pkt->data + 13, size);
                            res = ERR_SUCCESS;
                        }
                        else {
                            res = ERR_MALLOC;
                            hpcalcs_error("%s: couldn't allocate memory", __FUNCTION__);
                        }
                    }
                    // else do nothing. res is already 0.
                }
                else {
                    res = ERR_CALC_PACKET_FORMAT;
                    hpcalcs_warning("%s: unknown marker at beginning of image", __FUNCTION__);
                }
            }
            else {
                res = ERR_CALC_PACKET_FORMAT;
                hpcalcs_info("%s: packet is too short: %" PRIu32 "bytes", __FUNCTION__, pkt->size);
            }
            prime_vtl_pkt_del(pkt);
        }
        else {
            hpcalcs_error("%s: failed to read packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

// Seems to be made of a series of CMD_PRIME_RECV_FILE.
HPEXPORT int HPCALL calc_prime_s_send_file(calc_handle * handle, files_var_entry * file) {
    int res;
    if (handle != NULL && file != NULL) {
        uint8_t namelen = (uint8_t)char16_strlen(file->name) * 2;
        uint32_t size = 10 - 6 + namelen + file->size; // Size of the data after the header.
        prime_vtl_pkt * pkt = prime_vtl_pkt_new(size + 6); // Add size of the header.
        hpcalcs_debug("Virtual packet has size %" PRIu32 " (%" PRIx32 ")\n", size, size);
        if (pkt != NULL) {
            uint8_t * ptr;
            uint16_t crc16;

            pkt->cmd = CMD_PRIME_RECV_FILE;
            ptr = pkt->data;
//            *ptr++ = 0x00; // Report number
//            *ptr++ = 0x00; // ?
            *ptr++ = CMD_PRIME_RECV_FILE;
            *ptr++ = 0x01;
            *ptr++ = (uint8_t)((size >> 24) & 0xFF);
            *ptr++ = (uint8_t)((size >> 16) & 0xFF);
            *ptr++ = (uint8_t)((size >>  8) & 0xFF);
            *ptr++ = (uint8_t)((size      ) & 0xFF);
            *ptr++ = file->type;
            *ptr++ = namelen;
            *ptr++ = 0x00; // CRC16, set it to 0 for now.
            *ptr++ = 0x00;
            memcpy(ptr, file->name, namelen);
            ptr += namelen;
            memcpy(ptr, file->data, file->size);
            crc16 = crc16_block(pkt->data, size); // Yup, the last 6 bytes of the packet are excluded from the CRC.
            pkt->data[8] = crc16 & 0xFF;
            pkt->data[9] = (crc16 >> 8) & 0xFF;
            res = prime_send_data(handle, pkt);

            prime_vtl_pkt_del(pkt);
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_PARAMETER;
        hpcalcs_error("%s: an argument is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_r_send_file(calc_handle * handle) {
    int res;
    if (handle != NULL) {
        // There doesn't seem anything to do, beyond eliminating packets starting with 0xFF.
        res = calc_prime_r_check_ready(handle, NULL, NULL);
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

// Receiving an individual file from the calculator seems to start by a CMD_PRIME_REQ_FILE.
HPEXPORT int HPCALL calc_prime_s_recv_file(calc_handle * handle, files_var_entry * file) {
    int res;
    if (handle != NULL && file != NULL) {
        uint8_t namelen = (uint8_t)char16_strlen(file->name) * 2;
        uint32_t size = 10 - 6 + namelen; // Size of the data after the header.
        prime_vtl_pkt * pkt = prime_vtl_pkt_new(size + 6); // Add size of the header.
        if (pkt != NULL) {
            uint8_t * ptr;
            uint16_t crc16;

            pkt->cmd = CMD_PRIME_RECV_FILE;
            ptr = pkt->data;
//            *ptr++ = 0x00; // Report number
//            *ptr++ = 0x00; // ?
            *ptr++ = CMD_PRIME_REQ_FILE;
            *ptr++ = 0x01;
            *ptr++ = (uint8_t)((size >> 24) & 0xFF);
            *ptr++ = (uint8_t)((size >> 16) & 0xFF);
            *ptr++ = (uint8_t)((size >>  8) & 0xFF);
            *ptr++ = (uint8_t)((size      ) & 0xFF);
            *ptr++ = file->type;
            *ptr++ = namelen;
            *ptr++ = 0x00; // CRC16, set it to 0 for now.
            *ptr++ = 0x00;
            memcpy(ptr, file->name, namelen);
            crc16 = crc16_block(pkt->data, size); // Yup, the last 6 bytes are excluded from the CRC.
            pkt->data[8] = crc16 & 0xFF;
            pkt->data[9] = (crc16 >> 8) & 0xFF;
            res = prime_send_data(handle, pkt);
            
            prime_vtl_pkt_del(pkt);
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_PARAMETER;
        hpcalcs_error("%s: an argument is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_r_recv_file(calc_handle * handle, files_var_entry ** out_file) {
    int res;
    prime_vtl_pkt * pkt;
    // TODO: if no file was received, have *out_file = NULL, but res = 0.
    if (handle != NULL) {
        res = read_vtl_pkt(handle, CMD_PRIME_RECV_FILE, &pkt, 1);
        if (res == 0 && pkt != NULL) {
            if (pkt->size >= 11) {
                // Packet has CRC
                uint16_t computed_crc; // 0x0000 ?
                uint8_t * ptr = pkt->data;
                uint16_t embedded_crc = (((uint16_t)(ptr[9])) << 8) | ((uint16_t)(ptr[8]));
                // Reset CRC before computing
                ptr[8] = 0x00;
                ptr[9] = 0x00;
                computed_crc = crc16_block(ptr, pkt->size - 6); // The CRC contains the initial 0x00, but not the final 6 bytes (...).
                hpcalcs_info("%s: embedded=%" PRIX16 " computed=%" PRIX16, __FUNCTION__, embedded_crc, computed_crc);
                if (computed_crc != embedded_crc) {
                    hpcalcs_error("%s: CRC mismatch", __FUNCTION__);
                    // TODO: change res.
                }

                if (out_file != NULL) {
                    uint8_t namelen;
                    uint32_t size;

                    *out_file = NULL;
                    namelen = pkt->data[7];
                    size = pkt->size - 10 - namelen;

                    if (!(size & UINT32_C(0x80000000))) {
                        *out_file = hpfiles_ve_create_with_data(&pkt->data[10 + namelen], size);
                        if (*out_file != NULL) {
                            (*out_file)->type = pkt->data[6];
                            memcpy((*out_file)->name, &pkt->data[10], namelen);
                            hpcalcs_info("%s: created entry for %ls with size %" PRIu32 " and type %02X", __FUNCTION__, (*out_file)->name, (*out_file)->size, (*out_file)->type);
                        }
                        else {
                            res = ERR_MALLOC;
                            hpcalcs_error("%s: couldn't create entry", __FUNCTION__);
                        }
                    }
                    else {
                        res = ERR_CALC_PACKET_FORMAT;
                        hpcalcs_error("%s: weird size (packet too short ?)", __FUNCTION__);
                        // TODO: change res.
                    }
                }
            }
            else {
                if (pkt->data[0] != 0xF9) {
                    res = ERR_CALC_PACKET_FORMAT;
                    hpcalcs_info("%s: packet is too short: %" PRIu32 "bytes", __FUNCTION__, pkt->size);
                }
                else {
                    hpcalcs_info("%s: skipping F9 packet", __FUNCTION__);
                    if (out_file != NULL) {
                        *out_file = NULL;
                    }
                }
            }
            // else do nothing. res is already 0.
            prime_vtl_pkt_del(pkt);
        }
        else {
            hpcalcs_error("%s: failed to read packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_s_recv_backup(calc_handle * handle) {
    int res;
    if (handle != NULL) {
        prime_vtl_pkt * pkt = prime_vtl_pkt_new(2);
        if (pkt != NULL) {
            uint8_t * ptr;

            pkt->cmd = CMD_PRIME_RECV_FILE;
            ptr = pkt->data;
//            *ptr++ = 0x00; // Report number ?
//            *ptr++ = 0x00; // ?
            *ptr++ = CMD_PRIME_RECV_BACKUP;
            res = prime_send_data(handle, pkt);
            prime_vtl_pkt_del(pkt);
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create packet", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_HANDLE;
        hpcalcs_error("%s: handle is NULL", __FUNCTION__);
    }
    return res;
}

HPEXPORT int HPCALL calc_prime_r_recv_backup(calc_handle * handle, files_var_entry *** out_vars) {
    int res;
    if (handle != NULL) {
        uint32_t count = 0;
        files_var_entry ** entries = hpfiles_ve_create_array(count);
        if (entries != NULL) {
            for (;;) {
                res = calc_prime_r_recv_file(handle, &entries[count]);
                if (res == ERR_SUCCESS) {
                    if (entries[count] != NULL) {
                        files_var_entry ** new_entries;
                        hpcalcs_info("%s: continuing due to non-empty entry", __FUNCTION__);
                        count++;
                        new_entries = hpfiles_ve_resize_array(entries, count);
                        if (new_entries != NULL) {
                            entries = new_entries;
                            entries[count] = NULL;
                            if (out_vars != NULL) {
                                *out_vars = entries;
                            }
                        }
                        else {
                            res = ERR_MALLOC;
                            hpcalcs_error("%s: couldn't resize entries", __FUNCTION__);
                            hpfiles_ve_delete_array(entries);
                            if (out_vars != NULL) {
                                *out_vars = NULL;
                            }
                            break;
                        }
                    }
                    else {
                        hpcalcs_info("%s: breaking due to empty file", __FUNCTION__);
                        res = ERR_SUCCESS;
                        break;
                    }
                }
                else {
                    hpcalcs_error("%s: breaking due to reception failure", __FUNCTION__);
                    break;
                }
            }
        }
        else {
            res = ERR_MALLOC;
            hpcalcs_error("%s: couldn't create entries", __FUNCTION__);
        }
    }
    else {
        res = ERR_INVALID_PARAMETER;
        hpcalcs_error("%s: an argument is NULL", __FUNCTION__);
    }
    return res;
}