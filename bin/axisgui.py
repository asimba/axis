#!/usr/bin/env python
# -*- coding: utf-8 -*-

from platform import system
from subprocess import Popen,PIPE,STDOUT
if system()=="Windows":
    from subprocess import STARTUPINFO

try:
    import wx
    from wx.lib.embeddedimage import PyEmbeddedImage
except:
    _MSG="Please install wxPython 2.8 or higher (http://www.wxpython.org/)!\n\
Under Debian or Ubuntu you may try: sudo aptitude install python-wxgtk2.8"
    if system()=="Windows":
        import ctypes
        MessageBox=ctypes.windll.user32.MessageBoxW
        MessageBox(0,unicode(_MSG),u'Error!',16)
    if system()=="Linux":
        try:
            f=Popen(['notify-send',_MSG],False,stdin=None,stdout=None,\
            stderr=None)
            f.wait()
        except: pass
        try:
            f=Popen(['xmessage',_MSG],False,stdin=None,stdout=None,stderr=None)
            f.wait()
        except: pass
    raise RuntimeError(_MSG)

from threading import Thread
import signal
from sys import argv,executable
try:
    from sys import frozen
    if frozen==1: selfrozen=1
    else: selfrozen=0
except: selfrozen=0
import locale,pickle,base64,zlib
from os import getcwd,chdir,remove
from os.path import abspath,realpath,basename,dirname,expanduser,join,isfile
from sys import getfilesystemencoding
from textwrap import wrap,fill

class translator():
    def __init__(self):
        self.voc={}
        self.locale=locale.getdefaultlocale()
    def find(self,key):
        try: wxu=True if (wx.VERSION[0]>3 or osflag) else False
        except: wxu=False
        if key in self.voc:
            if self.locale[0] in self.voc[key]:
                if wxu: return self.voc[key][self.locale[0]]
                else:
                    return self.voc[key][self.locale[0]].encode(\
                        self.locale[1])
        return key

t=translator()

t.voc=pickle.loads(zlib.decompress(base64.decodestring("\
  eNrFV0tv1DAQvu+vMKflAiR+xTkhnhInUCs4IaE0cWikNo6cLKX/HnucrSePVbdsC5do13Hm8fmb\
  +cbPqy7ZnG/P9AvdDtqSruj7G2Or7aZLN8+rjrqXdvfj7KtbYJtv33cJT2v/ZBqe1D85jSs8gd8K\
  rQsStzK0lfHwIthL0Nfh9QWs1JuOb/r+fPt5N3S7gdTNlX5VNVaXg7G3pC2uNWl60pqB9J0um7rR\
  1TMXrfDhy83PfdQVioUz+FNExyxDv8NWTWImrAyxEPSdRnsFvM7w1/BkyBLnyEUeM2TJq4AGAi6J\
  VoO3QxAz9WzTZYDPu0tjeodFu0eJGEvugHKIKI9IHhEJZxdiSFEaB08wLMFWLvA5xrxHc/nBjDGo\
  Ya8if58+146nyXr6nsLA4ZT+55xdIAxifGvNTQ+BcQhMxMA0CinDybrNEj7+MpamZ/tgDOkvjR08\
  09MMjKk7YzTF8IIZtSBuueAyLr4RhAKdVn5MtZaOjmkO8b7Xpal8sjSBTpLGZPn8fCegqoVtuijO\
  EJJwxik4+9DunTFwxqOzUx2I0cGsPVIJjrIZt0ZTYuFuwqp7QHTWFXg9Hwo7kMoj2bQ/SWdNqfve\
  e8+9d5bM+1uCGlES/fISF9mjAM9SiPBrW1y4VjMYV3NtRd78duzUv3W5G/y6pyejECqLoaKaYvU8\
  +7GmahRpeVxn2RbO+Zbgtoz/4M0Yh8pRlgWJOdPX5pcmRetSaPrBQ26i6rz2yYCssKgrlCGDyaLu\
  MPpjF4EPeI6aT4oIQtEejXZOavAIAboXrtcumSAcH6w1Fk4KFILls640Ng+FOtSonA45Hrrvp8rN\
  D057y2JoTOu6cG3sdfh9U/TEArAVqa25JsPlrE1zaNOcrjLkRKWZyH66MIW7v8TnhmaWFaaWC41P\
  cJHjWl7MBDmWwApFj5twiYxKLPcOLYb1buQn1nkO4sLFCap3DMFGjp6o3lyOjc50a32Og7ZxNack\
  LplkATE9UIGP2f94ULlP7YOGUpBCkT7JWHpE0fzTsVTQFaKOJS9ApgV/Yo7eP5mJIO9fAuP8oRVX\
  VhfVLem99o7HBlovkNbXi4FJLhp6ig9V4tDxqdbrogCyJBRGsJhUuYAJQCYnIPjXRStTNJ3odlG1\
  EiRfsgdOJydWpAwi/h6BJMMtUM5Hz4fm7C6hMpuexUhkCZopT7hWrfMyS+76omdgBhKZ0dmMjwNm\
  tb8KMjzINIdV+U6Jp3WZgXZkYr1BMXwruEAzysEjpY9Xq0+gyKPPSTlgak02OXCQVq2wPgOtyh5L\
  q06shiyP9xY3ee1vLq6/WQ3CFMhRFleeXwqESaX33mZEegifI242+7v+waGMzm3gQxm5Rh3NVZCW\
  Ny6VW7NzV+GdhfFcgayopazgVkwXRSlwNm4wVkEUPoaaUND9Vez+dLVilW/WL/8A8ah6pA==\
")))

def _(s): return t.find(s)

axislogo = PyEmbeddedImage(
    "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"
    "EZc0MPcAAAAASUVORK5CYII=")

class processing(Thread):
    def __init__(self,argframe,argcmd,argcwd,\
        argfields,argelements,argcheckbox,argstart,argstop,spane):
        Thread.__init__(self)
        self.mainframe=argframe
        self.cmd=argcmd
        self.fields=argfields
        self.elements=argelements
        self.checkbox=argcheckbox
        self.start_button=argstart
        self.stop_button=argstop
        self.stop_flag=False
        self.cwd_path=argcwd
        self.s_pane=spane

    def run(self):
        cwd=abspath(getcwd())
        try:
            wx.CallAfter(self.start_button.Enable,False)
            wx.CallAfter(self.stop_button.Enable,True)
            wx.CallAfter(self.mainframe.status_panel.Clear)
            wx.CallAfter(self.s_pane.Disable)
            for element in self.elements: wx.CallAfter(element.Enable,False)
            for field in self.fields: wx.CallAfter(field.Disable)
            wx.CallAfter(self.checkbox.Disable)
            chdir(self.cwd_path)
            if system()=="Windows":
                si=STARTUPINFO()
                si.dwFlags|=1
                si.wShowWindow=0
                proc=Popen(self.cmd,shell=False,stdin=None,stdout=PIPE,\
                    stderr=STDOUT,bufsize=0,startupinfo=si)
            else:
                proc=Popen(self.cmd,shell=False,stdin=None,stdout=PIPE,\
                    stderr=STDOUT,bufsize=0)
            cr=0
            line_start=0
            line_stop=0
            while True:
                if self.stop_flag:
                    if system()=="Linux": proc.send_signal(signal.SIGKILL)
                    else: proc.kill()
                    wx.CallAfter(self.mainframe.status_panel.Clear)
                    break
                symbol=proc.stdout.read(1)
                if not symbol: break
                else:
                    symbol_code=ord(symbol)
                    if symbol_code!=0xd:
                        if cr==0xd and symbol_code!=0xa:
                            cr=symbol_code
                            wx.CallAfter(self.mainframe.status_panel.Remove,\
                                line_start,line_stop)
                            line_stop=line_start
                        if system()=="Windows" and cr==0xd\
                            and symbol_code==0xa:
                            line_stop+=1
                        wx.CallAfter(self.mainframe.status_panel.write,symbol)
                        line_stop+=1
                        if ord(symbol)==0xa: line_start=line_stop
                    else: cr=0xd
                    wx.CallAfter(self.mainframe.status_panel.Refresh)
            proc.wait()
            for field in self.fields:
                wx.CallAfter(field.Clear)
                wx.CallAfter(field.Enable)
            for element in self.elements: wx.CallAfter(element.Enable,True)
            wx.CallAfter(self.checkbox.Enable)
            chdir(cwd)
            wx.CallAfter(self.checkbox.SetValue,False)
            wx.CallAfter(self.start_button.Enable,True)
            wx.CallAfter(self.stop_button.Enable,False)
            wx.CallAfter(self.s_pane.Enable)
            wx.CallAfter(self.mainframe.RequestUserAttention)
        except: chdir(cwd)

class main_frame(wx.Frame):
    def chooser(self,event,fn,flags,field,msg):
        filename=""
        dlg=fn(self,message=_(msg),style=flags)
        if dlg.ShowModal()==wx.ID_OK: filename=dlg.GetPath()
        dlg.Destroy()
        if filename:
            field.Clear()
            field.WriteText(filename)
            field.Refresh()

    def __init__(self,*args,**kwds):
        kwds["style"]=wx.CAPTION|wx.SYSTEM_MENU|wx.RESIZE_BORDER|\
            wx.CLOSE_BOX|wx.MINIMIZE_BOX|wx.MAXIMIZE_BOX|\
            wx.FULL_REPAINT_ON_RESIZE
        wx.Frame.__init__(self,*args,**kwds)
        font=wx.Font(10,wx.MODERN,wx.NORMAL,wx.NORMAL,False,u'Consolas')
        self.modes_notebook=wx.Notebook(self,-1,style=0)
        self.decode_pane=wx.Panel(self.modes_notebook,-1)
        self.encode_pane=wx.Panel(self.modes_notebook,-1)
        self.choose_frame_sizer_1_staticbox=wx.StaticBox(self.encode_pane,\
            -1,_("Choose output file"))
        self.password_sizer_0_staticbox=wx.StaticBox(self.encode_pane,-1,\
            _("Enter password"))
        self.password_sizer_1_staticbox=wx.StaticBox(self.encode_pane,-1,\
            _("Re-enter password"))
        self.choose_frame_sizer_2_staticbox=wx.StaticBox(self.decode_pane,\
            -1,_("Choose input file"))
        self.choose_frame_sizer_3_staticbox=wx.StaticBox(self.decode_pane,\
            -1,_("Choose output directory"))
        self.password_sizer_3_staticbox=wx.StaticBox(self.decode_pane,-1,\
            _("Enter password"))
        self.password_sizer_4_staticbox=wx.StaticBox(self.decode_pane,-1,\
            _("Re-enter password"))
        self.choose_frame_sizer_0_staticbox=wx.StaticBox(self.encode_pane,\
            -1,_("Choose input file or directory"))
        self.choose_field_0=wx.TextCtrl(self.encode_pane,-1,"")
        self.browse_0=wx.Button(self.encode_pane,-1,_("File"),\
            size=(100,-1))
        self.browse_1=wx.Button(self.encode_pane,-1,_("Directory"),\
            size=(100,-1))
        self.choose_field_1=wx.TextCtrl(self.encode_pane,-1,"")
        self.browse_2=wx.Button(self.encode_pane,-1,_("Browse"),\
            size=(200,-1))
        self.jam_checkbox_0=wx.CheckBox(self.encode_pane,-1,\
            _("Remove identification information from the output file"))
        self.password_field_0=wx.TextCtrl(self.encode_pane,-1,"",\
            style=wx.TE_PASSWORD)
        self.password_field_1=wx.TextCtrl(self.encode_pane,-1,"",\
            style=wx.TE_PASSWORD)
        self.start_button_0=wx.Button(self.encode_pane,-1,\
            _("Start encoding process"))
        self.stop_button_0=wx.Button(self.encode_pane,-1,\
            _("Stop encoding process"))
        self.choose_field_2=wx.TextCtrl(self.decode_pane,-1,"")
        self.browse_3=wx.Button(self.decode_pane,-1,_("Browse"),\
            size=(100,-1))
        self.choose_field_3=wx.TextCtrl(self.decode_pane,-1,"")
        self.browse_4=wx.Button(self.decode_pane,-1,_("Browse"),\
            size=(100,-1))
        self.jam_checkbox_1=wx.CheckBox(self.decode_pane,-1,\
            _("Identification information was removed from the input file"))
        self.password_field_2=wx.TextCtrl(self.decode_pane,-1,"",\
            style=wx.TE_PASSWORD)
        self.password_field_3=wx.TextCtrl(self.decode_pane,-1,"",\
            style=wx.TE_PASSWORD)
        self.start_button_1=wx.Button(self.decode_pane,-1,\
            _("Start decoding process"))
        self.stop_button_1=wx.Button(self.decode_pane,-1,\
            _("Stop decoding process"))
        self.status_panel=wx.TextCtrl(self,-1,"",style=wx.TE_MULTILINE|\
            wx.TE_READONLY|wx.HSCROLL|wx.TE_WORDWRAP)
        self.status_panel.SetFont(font)
        if selfrozen==1: self.selfpath=dirname(realpath(executable))
        else: self.selfpath=dirname(realpath(argv[0]))
        self.Bind(wx.EVT_BUTTON,\
            lambda event: self.chooser(event,wx.FileDialog,wx.FD_OPEN,\
            self.choose_field_0,"Choose a file"),self.browse_0)
        self.Bind(wx.EVT_BUTTON,\
            lambda event: self.chooser(event,wx.DirDialog,wx.FD_OPEN,\
            self.choose_field_0,"Choose a directory"),self.browse_1)
        self.Bind(wx.EVT_BUTTON,\
            lambda event: self.chooser(event,wx.FileDialog,wx.FD_SAVE|\
            wx.FD_OVERWRITE_PROMPT,self.choose_field_1,"Choose a file"),\
            self.browse_2)
        self.Bind(wx.EVT_BUTTON,self.OnStartEncodingClicked,\
            self.start_button_0)
        self.Bind(wx.EVT_BUTTON,self.OnStopClicked,self.stop_button_0)
        self.Bind(wx.EVT_BUTTON,\
            lambda event: self.chooser(event,wx.FileDialog,wx.FD_OPEN,\
            self.choose_field_2,"Choose a file"),self.browse_3)
        self.Bind(wx.EVT_BUTTON,\
            lambda event: self.chooser(event,wx.DirDialog,wx.FD_OPEN,\
            self.choose_field_3,"Choose a directory"),self.browse_4)
        self.Bind(wx.EVT_BUTTON,self.OnStartDecodingClicked,\
            self.start_button_1)
        self.Bind(wx.EVT_BUTTON,self.OnStopClicked,self.stop_button_1)
        self.axispath=join(self.selfpath,'axis')
        self.thread=None
        self.__set_properties()
        self.__do_layout()

    def __set_properties(self):
        self.SetTitle("Axis Encode/Decode GUI")
        self.SetMinSize((800,560))
        self.choose_field_0.SetMinSize((420,-1))
        self.choose_field_1.SetMinSize((420,-1))
        self.stop_button_0.Enable(False)
        self.choose_field_2.SetMinSize((420,-1))
        self.choose_field_3.SetMinSize((420,-1))
        self.stop_button_1.Enable(False)
        self.status_panel.SetMinSize((-1,210))
        if system()=="Linux": self._icon=axislogo.GetIcon()
        if system()=="Windows":
            temp_image=axislogo.GetImage()
            temp_image.Rescale(32,32,wx.IMAGE_QUALITY_HIGH)
            self._icon=wx.Icon()
            self._icon.CopyFromBitmap(wx.Bitmap(temp_image))
        try: self.SetIcon(self._icon)
        except: pass

    def __do_layout(self):
        encode_pane_sizer=wx.BoxSizer(wx.VERTICAL)
        choose_frame_sizer_0=wx.StaticBoxSizer(\
            self.choose_frame_sizer_0_staticbox,wx.HORIZONTAL)
        choose_frame_sizer_0.Add(self.choose_field_0,1,wx.EXPAND,0)
        choose_frame_sizer_0.Add(self.browse_0,0,wx.FIXED_MINSIZE|wx.EXPAND,0)
        choose_frame_sizer_0.Add(self.browse_1,0,wx.FIXED_MINSIZE|wx.EXPAND,0)
        encode_pane_sizer.Add(choose_frame_sizer_0,1,wx.EXPAND,0)
        choose_frame_sizer_1=wx.StaticBoxSizer(\
            self.choose_frame_sizer_1_staticbox,wx.HORIZONTAL)
        choose_frame_sizer_1.Add(self.choose_field_1,1,wx.EXPAND,0)
        choose_frame_sizer_1.Add(self.browse_2,0,wx.EXPAND,0)
        encode_pane_sizer.Add(choose_frame_sizer_1,1,wx.EXPAND,0)
        encode_pane_sizer.Add((0,10),0,0,0)
        encode_pane_sizer.Add(self.jam_checkbox_0,0,wx.EXPAND,0)
        encode_pane_sizer.Add((0,10),0,0,0)
        password_field_sizer_0=wx.StaticBoxSizer(self.password_sizer_0_staticbox,\
            wx.HORIZONTAL)
        password_field_sizer_0.Add(self.password_field_0,1,wx.EXPAND,0)
        encode_pane_sizer.Add(password_field_sizer_0,1,wx.EXPAND,0)
        password_field_sizer_1=wx.StaticBoxSizer(self.password_sizer_1_staticbox,\
            wx.HORIZONTAL)
        password_field_sizer_1.Add(self.password_field_1,1,wx.EXPAND,0)
        encode_pane_sizer.Add(password_field_sizer_1,1,wx.EXPAND,0)
        encode_pane_sizer.Add(self.start_button_0,0,wx.EXPAND|\
            wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL,0)
        encode_pane_sizer.Add(self.stop_button_0,0,wx.EXPAND|\
            wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL,0)
        self.encode_pane.SetSizer(encode_pane_sizer)
        decode_pane_sizer=wx.BoxSizer(wx.VERTICAL)
        choose_frame_sizer_2=wx.StaticBoxSizer(\
            self.choose_frame_sizer_2_staticbox,wx.HORIZONTAL)
        choose_frame_sizer_2.Add(self.choose_field_2,1,wx.EXPAND,0)
        choose_frame_sizer_2.Add(self.browse_3,0,wx.FIXED_MINSIZE|wx.EXPAND,0)
        decode_pane_sizer.Add(choose_frame_sizer_2,1,wx.EXPAND,0)
        choose_frame_sizer_3=wx.StaticBoxSizer(\
            self.choose_frame_sizer_3_staticbox,wx.HORIZONTAL)
        choose_frame_sizer_3.Add(self.choose_field_3,1,wx.EXPAND,0)
        choose_frame_sizer_3.Add(self.browse_4,0,wx.FIXED_MINSIZE|wx.EXPAND,0)
        decode_pane_sizer.Add(choose_frame_sizer_3,1,wx.EXPAND,0)
        decode_pane_sizer.Add((0,10),0,0,0)
        decode_pane_sizer.Add(self.jam_checkbox_1,0,wx.EXPAND,0)
        decode_pane_sizer.Add((0,10),0,0,0)
        password_field_sizer_2=wx.StaticBoxSizer(self.password_sizer_3_staticbox,\
            wx.HORIZONTAL)
        password_field_sizer_2.Add(self.password_field_2,1,wx.EXPAND,0)
        decode_pane_sizer.Add(password_field_sizer_2,1,wx.EXPAND,0)
        password_field_sizer_3=wx.StaticBoxSizer(self.password_sizer_4_staticbox,\
            wx.HORIZONTAL)
        password_field_sizer_3.Add(self.password_field_3,1,wx.EXPAND,0)
        decode_pane_sizer.Add(password_field_sizer_3,1,wx.EXPAND,0)
        decode_pane_sizer.Add(self.start_button_1,0,wx.EXPAND|\
            wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL,0)
        decode_pane_sizer.Add(self.stop_button_1,0,wx.EXPAND|\
            wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL,0)
        self.decode_pane.SetSizer(decode_pane_sizer)
        self.modes_notebook.AddPage(self.encode_pane,_("Encode"))
        self.modes_notebook.AddPage(self.decode_pane,_("Decode"))
        base_frame_sizer=wx.BoxSizer(wx.VERTICAL)
        base_frame_sizer.Add(self.modes_notebook,0,wx.EXPAND,0)
        base_frame_sizer.Add(self.status_panel,1,wx.EXPAND,0)
        self.SetSizer(base_frame_sizer)
        self.Layout()
        self.Fit()
        self.Centre()

    def CheckAxis(self):
        try:
            f=Popen([self.axispath,'-v'],shell=False,stdin=None,stdout=PIPE,
                stderr=STDOUT)
            f.wait()
            lines=f.stdout.readlines()
            f.stdout.close()
            for l in lines:
                if l[0:8]=='Axis 0.8':
                    for l in lines:
                        for ll in wrap(l,78):
                            self.status_panel.WriteText(ll+'\n')
                    return
        except: pass
        wx.MessageBox(_("Unable to find Axis executable!"),_("Error!"),wx.OK|\
            wx.ICON_ERROR)
        self.Close()

    def OnStartEncodingClicked(self,event):
        if self.thread!=None and self.thread.is_alive():
            wx.MessageBox(_("Process is already started!"),_("Error!"),wx.OK|\
                wx.ICON_ERROR)
            return
        if len(self.choose_field_0.GetValue())==0:
            wx.MessageBox(_("Input file/directory name is not specified!"),\
                _("Error!"),wx.OK|wx.ICON_ERROR)
            return
        if len(self.choose_field_1.GetValue())==0:
            wx.MessageBox(_("Output file/directory name is not specified!"),\
                _("Error!"),wx.OK|wx.ICON_ERROR)
            return
        if self.password_field_0.GetValue()!=self.password_field_1.GetValue():
            wx.MessageBox(_("Entered passwords are not identical!"),\
                _("Error!"),wx.OK|wx.ICON_ERROR)
            self.password_field_0.Clear()
            self.password_field_1.Clear()
            return
        if len(self.password_field_0.GetValue())<6:
            wx.MessageBox(_("Password is too short!"),_("Error!"),wx.OK|\
                wx.ICON_ERROR)
            self.password_field_0.Clear()
            self.password_field_1.Clear()
            return
        output=self.choose_field_1.GetValue().encode(getfilesystemencoding())
        cmd=[self.axispath,'-e','-s','-i',basename(\
            abspath(\
            self.choose_field_0.GetValue())).encode(getfilesystemencoding()),\
            '-o',output,\
            '-p',\
            self.password_field_0.GetValue().encode(getfilesystemencoding())]
        if self.jam_checkbox_0.IsChecked():
            cmd+=['-j']
        fields=(self.choose_field_0,self.choose_field_1,\
            self.password_field_0,self.password_field_1)
        elements=(self.browse_0,self.browse_1,self.browse_2)
        if isfile(output):
            ret=wx.MessageBox(_("Remove an existing output file?"),_("Stop!"),\
                wx.YES_NO|wx.NO_DEFAULT|wx.ICON_EXCLAMATION)
            if ret==wx.YES:
                try: remove(output)
                except: pass
            else: return
        self.thread=processing(self,cmd,dirname(\
            abspath(self.choose_field_0.GetValue())),\
            fields,elements,self.jam_checkbox_0,self.start_button_0,\
            self.stop_button_0,self.decode_pane)
        self.thread.start()

    def OnStartDecodingClicked(self,event):
        if self.thread!=None and self.thread.is_alive():
            wx.MessageBox(_("Process is already started!"),_("Error!"),wx.OK|\
                wx.ICON_ERROR)
            return
        if len(self.choose_field_2.GetValue())==0:
            wx.MessageBox(_("Input file/directory name is not specified!"),\
                _("Error!"),wx.OK|wx.ICON_ERROR)
            return
        if len(self.choose_field_3.GetValue())==0:
            wx.MessageBox(_("Output file/directory name is not specified!"),\
                _("Error!"),wx.OK|wx.ICON_ERROR)
            return
        if self.password_field_2.GetValue()!=self.password_field_3.GetValue():
            wx.MessageBox(_("Entered passwords are not identical!"),\
                _("Error!"),wx.OK|wx.ICON_ERROR)
            self.password_field_2.Clear()
            self.password_field_3.Clear()
            return
        if len(self.password_field_2.GetValue())<6:
            wx.MessageBox(_("Password is too short!"),_("Error!"),\
                wx.OK|wx.ICON_ERROR)
            self.password_field_2.Clear()
            self.password_field_3.Clear()
            return
        cmd=[self.axispath,'-d','-n','-s','-i',\
            abspath(\
            self.choose_field_2.GetValue()).encode(getfilesystemencoding()),\
            '-p',\
            self.password_field_2.GetValue().encode(getfilesystemencoding())]
        if self.jam_checkbox_1.IsChecked(): cmd+=['-j']
        fields=(self.choose_field_2,self.choose_field_3,\
            self.password_field_2,self.password_field_3)
        elements=(self.browse_3,self.browse_4)
        self.thread=processing(self,cmd,\
            abspath(self.choose_field_3.GetValue()),\
            fields,elements,self.jam_checkbox_1,self.start_button_1,\
            self.stop_button_1,self.encode_pane)
        self.thread.start()

    def OnStopClicked(self,event):
        ret=wx.MessageBox(_("Are you shure?"),_("Stop!"),wx.YES_NO|\
            wx.NO_DEFAULT|wx.ICON_EXCLAMATION)
        if ret==wx.YES: self.thread.stop_flag=True
        else: return

class axisgui(wx.App):
    def OnInit(self):
        if wx.VERSION[0]<3: wx.InitAllImageHandlers()
        base_frame = main_frame(None,-1,"")
        self.SetTopWindow(base_frame)
        base_frame.CheckAxis()
        chdir(expanduser('~'))
        base_frame.Show()
        return 1

if __name__ == "__main__":
    axisgui = axisgui(0)
    axisgui.MainLoop()
