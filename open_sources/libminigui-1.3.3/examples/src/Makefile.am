COMMON_PROGS=helloworld mycontrol dialogbox input bmpbkgnd simplekey \
                static button listbox edit combobox menubutton progressbar trackbar newtoolbar propsheet \
                timeeditor listview spinbox monthcal coolbar treeview \
                scrollbar painter capture bitblt stretchblt loadbmp drawicon \
                createicon caretdemo cursordemo worldtime skindemo

if LITE_VERSION
if STAND_ALONE
LITE_PROGS=
else
LITE_PROGS=scrnsaver mginit
endif
else
LITE_PROGS=
endif

noinst_PROGRAMS=$(COMMON_PROGS) $(LITE_PROGS)

helloworld_SOURCES=helloworld.c
mycontrol_SOURCES=mycontrol.c
dialogbox_SOURCES=dialogbox.c
input_SOURCES=input.c
bmpbkgnd_SOURCES=bmpbkgnd.c
bitblt_SOURCES=bitblt.c
stretchblt_SOURCES=stretchblt.c
loadbmp_SOURCES=loadbmp.c
caretdemo_SOURCES=caretdemo.c
cursordemo_SOURCES=cursordemo.c
createicon_SOURCES=createicon.c
drawicon_SOURCES=drawicon.c
simplekey_SOURCES=simplekey.c
capture_SOURCES=capture.c
painter_SOURCES=painter.c
scrollbar_SOURCES=scrollbar.c
static_SOURCES=static.c
button_SOURCES=button.c
listbox_SOURCES=listbox.c
edit_SOURCES=edit.c
combobox_SOURCES=combobox.c
menubutton_SOURCES=menubutton.c
progressbar_SOURCES=progressbar.c
trackbar_SOURCES=trackbar.c
newtoolbar_SOURCES=newtoolbar.c
propsheet_SOURCES=propsheet.c
listview_SOURCES=listview.c
spinbox_SOURCES=spinbox.c
monthcal_SOURCES=monthcal.c
coolbar_SOURCES=coolbar.c
timeeditor_SOURCES=timeeditor.c
treeview_SOURCES=treeview.c
worldtime_SOURCES=worldtime.c timezone.c worldtime.h

mginit_SOURCES=mginit.c
scrnsaver_SOURCES=scrnsaver.c
skindemo_SOURCES=skindemo.c

listview_LDADD=-lmgext
spinbox_LDADD=-lmgext
monthcal_LDADD=-lmgext
coolbar_LDADD=-lmgext
timeeditor_LDADD=-lmgext
treeview_LDADD=-lmgext
skindemo_LDADD=-lmgext

EXTRA_DIST=bkgnd.jpg new2.jpg myicon.ico question.ico \
close.png  pause.png  progress-bk.png  stop.png     timer.png      volume.png \
main.png   play.png   progress.png     sysmenu.png  volume-bk.png


