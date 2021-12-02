#ifndef GUI_IAL_ORION_H
#define GUI_IAL_ORION_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

BOOL    InitOrionInput (INPUT* input, const char* mdev, const char* mtype);
void    TermOrionInput (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_IAL_ORION_H */


