import sys
import gdb

# Update module path.
dir_ = '/opt/crosstool/gcc-3.4.6-glibc-2.3.6/arm-linux/arm-linux/share/glib-2.0/gdb'
if not dir_ in sys.path:
    sys.path.insert(0, dir_)

from gobject import register
register (gdb.current_objfile ())
