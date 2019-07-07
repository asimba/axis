import zlib,base64,shutil

i=open('axisgui.py','rb')
o=open('axisgui','wb')
o.write('#!/usr/bin/env python\n# -*- coding: utf-8 -*-\nimport base64,zlib;\
exec compile(zlib.decompress(base64.decodestring(\"\\\n')
o.write(base64.encodestring(zlib.compress(i.read(),9)).replace('\n','\\\n'))
o.write('\")),"","exec")\n')
o.close()
i.close()
shutil.copy('axisgui','axisgui.pyw')
