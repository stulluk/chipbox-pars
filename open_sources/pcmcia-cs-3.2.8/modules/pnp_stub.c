/*======================================================================

    Stub for free-standing PnP BIOS support module

    pnp_stub.c 1.3 2003/12/29 09:19:01

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.

    The initial developer of the original code is David A. Hinds
    <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
    are Copyright (C) 2003 David A. Hinds.  All Rights Reserved.

    Alternatively, the contents of this file may be used under the
    terms of the GNU General Public License version 2 (the "GPL"), in
    which case the provisions of the GPL are applicable instead of the
    above.  If you wish to allow the use of your version of this file
    only under the terms of the GPL and not to allow others to use
    your version of this file under the MPL, indicate your decision
    by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL.  If you do not delete
    the provisions above, a recipient may use your version of this
    file under either the MPL or the GPL.
    
======================================================================*/

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pnp_bios.h>

MODULE_AUTHOR("David Hinds <dahinds@users.sourceforge.net>");
MODULE_DESCRIPTION("PnP BIOS interface");
MODULE_LICENSE("Dual MPL/GPL");

void pnp_bios_init(void);
void pnp_proc_init(void);
void pnp_proc_done(void);

int init_module(void)
{
    pnp_bios_init();
    pnp_proc_init();
    return 0;
}

int cleanup_module(void)
{
    pnp_proc_done();
}
