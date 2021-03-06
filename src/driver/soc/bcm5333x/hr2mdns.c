/*
 * $Id: hr2mdns.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: Copyright 2016 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 */

#include "system.h"

#ifdef CFG_ZEROCONF_MDNS_INCLUDED

#include "soc/bcm5333x.h"
#include "appl/mdns.h"

#define MDNS_LISTEN_PORT (5353)

/* Function:
 *   bcm5333x_mdns_enable_set
 * Description:
 *   Enable/Disable copying MDNS packets to CPU
 * Parameters:
 *   unit :
 *   mdns_enable :
 * Returns:
 *   None
 */
sys_error_t
bcm5333x_mdns_enable_set(uint8 unit, BOOL mdns_enable)
{
    uint32 tcam_entry[15], xy_entry[15];

    /* Action  : Copy to CPU */
    uint32 policy_entry[8] = {0x00000000, 0x02000000, 0x00000000, 0x00000110,
                              0x80004000, 0x04000000, 0x00000000, 0x00000000};
    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffff80, 0x7ff };

    if (unit >= BOARD_NUM_OF_UNITS){
        return SYS_ERR_PARAMETER;
    }

    sal_memset(tcam_entry, 0, sizeof(tcam_entry));

    if (mdns_enable) {
        /* Program L4 Dst port for MDNS at entry 127
         * FP_TCAM : L4_DST starts from bit 88
         */
        tcam_entry[0] = 0x3;
        tcam_entry[2] = ((MDNS_LISTEN_PORT << 24) & 0xFF000000);
        tcam_entry[3] = ((MDNS_LISTEN_PORT >> 8) & 0x000000FF);
        /* MASK */
        tcam_entry[10] = 0x0003fffc;

        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
        bcm5333x_mem_set(0, M_FP_TCAM(1), xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_POLICY_TABLE(1), policy_entry, 8);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(1),
                                                global_tcam_mask_entry, 3);
    } else {
        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
        bcm5333x_mem_set(0, M_FP_TCAM(1), xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(1),
                                                global_tcam_mask_entry, 3);
    }

    return SYS_OK;
}
/* Function:
 *   bcm533xx_mdns_enable_get
 * Description:
 *   Retrieve status of copying MDNS packets to CPU
 * Parameters:
 *   unit :
 *   mdns_enable :
 * Returns:
 *   None
 */
sys_error_t
bcm5333x_mdns_enable_get(uint8 unit, BOOL *mdns_enable)
{
    uint32 tcam_entry[15], dm_entry[15];

    if (unit >= BOARD_NUM_OF_UNITS){
        return SYS_ERR_PARAMETER;
    }

    bcm5333x_mem_get(0, M_FP_TCAM(1), dm_entry, 15);
    bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

    /* Check MDNS port */
    if (0x0 == tcam_entry[0]) {
        *mdns_enable = FALSE;
    } else {
        *mdns_enable = TRUE;
    }

    return SYS_OK;
}
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

