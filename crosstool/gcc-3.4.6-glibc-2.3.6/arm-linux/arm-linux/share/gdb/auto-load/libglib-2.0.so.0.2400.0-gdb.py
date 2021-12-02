import sys
import gdb

# Update module path.
dir = '/opt/crosstool/gcc-3.4.6-glibc-2.3.6/arm-linux/arm-linux/share/glib-2.0/gdb'
if not dir in sys.path:
    sys.path.insert(0, dir)

from glib import register
register (gdb.current_objfile ())
