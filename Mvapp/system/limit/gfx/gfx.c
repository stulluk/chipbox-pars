#include "linuxos.h"
#include <directfb.h>

#include "gfx.h"

#define DFBCHECK


static IDirectFB *dfb = NULL;
static IDirectFBSurface *primary = NULL;

tCS_GFX_Error_t     CS_GFX_Init(void)
{

    DFBSurfaceDescription dsc;
    
     DFBCHECK (DirectFBInit (0, NULL));
     DFBCHECK (DirectFBCreate (&dfb));
     DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));
     dsc.flags = DSDESC_CAPS;
     dsc.caps  = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    
    DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));

    return(CS_GFX_NO_ERROR);
}

tCS_GFX_Error_t     CS_GFX_SetColor(U8 r, U8 g, U8 b, U8 a)
{
    if(primary!=NULL)
        {
            DFBCHECK (primary->SetColor(primary, r, g,  b, a));
            return(CS_GFX_NO_ERROR);
        }
    else
        {
            return(CS_GFX_ERROR);
        }
        
}


tCS_GFX_Error_t     CS_GFX_DrawLine(U16 x1, U16 y1, U16 x2, U16 y2)
{
    if(primary!=NULL)
        {
            DFBCHECK (primary->DrawLine(primary, x1, y1,  x2, y2));
            //DFBCHECK (primary->Flip (primary, NULL, 0));
            return(CS_GFX_NO_ERROR);
        }
    else
        {
            return(CS_GFX_ERROR);
        }
        
}

tCS_GFX_Error_t     CS_GFX_FillRectangle(U16 x, U16 y, U16 w, U16 h)
{
    if(primary!=NULL)
        {
            DFBCHECK (primary->FillRectangle(primary, x, y,  w, h));
            //DFBCHECK (primary->Flip (primary, NULL, 0));
            return(CS_GFX_NO_ERROR);
        }
    else
        {
            return(CS_GFX_ERROR);
        }
        
}

tCS_GFX_Error_t     CS_GFX_DrawRectangle(U16 x, U16 y, U16 w, U16 h)
{
    if(primary!=NULL)
        {
            DFBCHECK (primary->DrawRectangle(primary, x, y,  w, h));
            //DFBCHECK (primary->Flip (primary, NULL, 0));
            return(CS_GFX_NO_ERROR);
        }
    else
        {
            return(CS_GFX_ERROR);
        }
        
}

tCS_GFX_Error_t     CS_GFX_DrawDot(U16 x, U16 y)
{
    if(primary!=NULL)
        {
            DFBCHECK (primary->DrawLine(primary, x, y, x, y));
            //DFBCHECK (primary->Flip (primary, NULL, 0));
            return(CS_GFX_NO_ERROR);
        }
    else
        {
            return(CS_GFX_ERROR);
        }
        
}



