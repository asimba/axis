#!/usr/bin/env python
# -*- coding: utf-8 -*-

import locale,pickle,base64,zlib
from platform import system

class translator():
    def __init__(self):
        self.voc={}
        self.locale=locale.getdefaultlocale()

    def find(self,key):
        if self.voc.has_key(key):
            if self.voc[key].has_key(self.locale[0]):
                return self.voc[key][self.locale[0]].encode(self.locale[1])
        return key

def add(tr,key,language,translation):
    if tr.voc.has_key(key):
        tr.voc[key][language]=unicode(translation,'utf-8')
    else:
        tr.voc[key]={language:unicode(translation,'utf-8')}

def load(tr,data):
    if type(data)==tuple and len(data):
        for l in data:
            if type(l)==tuple and len(l)==3:
                add(tr,l[0],l[1],l[2])
            else:
                return

def dumpvoc(tr):
    return base64.encodestring(zlib.compress(pickle.dumps(t.voc),9)).\
        replace('\n','\\\n')

def loadvoc(tr,data):
    tr.voc=pickle.loads(zlib.decompress(base64.decodestring(data)))

t=translator()

load(t,(\
("Choose output file","ru_RU","Выберите конечный файл"),\
("Enter password","ru_RU","Введите пароль"),\
("Re-enter password","ru_RU","Повторите ввод пароля"),\
("Choose input file","ru_RU","Выберите исходный файл"),\
("Choose output directory","ru_RU","Выберите конечную директорию"),\
("Choose input file or directory","ru_RU",\
    "Выберите исходный файл или директорию"),\
("File","ru_RU","Файл"),\
("Directory","ru_RU","Директория"),\
("Browse","ru_RU","Обзор"),\
("Remove identification information from the output file","ru_RU",\
    "Не включать в конечный файл вспомогательные данные"),\
("Start encoding process","ru_RU","Начать кодирование"),\
("Stop encoding process","ru_RU","Остановить кодирование"),\
("Identification information was removed from the input file","ru_RU",\
    "Исходный файл не содержит вспомогательной информации"),\
("Start decoding process","ru_RU","Начать декодирование"),\
("Stop decoding process","ru_RU","Остановить декодирование"),\
("Choose a file","ru_RU","Выберите файл"),\
("Choose a directory","ru_RU","Выберите директорию"),\
("Encode","ru_RU","Кодирование"),\
("Decode","ru_RU","Декодирование"),\
("Unable to find Axis executable!","ru_RU",\
    "Исполняемый файл 'axis' не найден!"),\
("Process is already started!","ru_RU","Процесс уже запущен!"),\
("Input file/directory name is not specified!","ru_RU",\
    "Не указано имя исходного файла/директории!"),\
("Output file/directory name is not specified!","ru_RU",\
    "Не указано имя конечного файла/директории!"),\
("Entered passwords are not identical!","ru_RU",\
    "Введённые пароли не совпадают!"),\
("Password is too short!","ru_RU","Слишком короткий пароль!"),\
("Remove an existing output file?","ru_RU",\
    "Удалить существующий конечный файл?"),\
("Are you shure?","ru_RU","Вы уверены?"),\
("Error!","ru_RU","Ошибка!"),\
("Stop!","ru_RU","Стоп!")\
))

def _(s):
    return t.find(s)

import pickle,base64,zlib

if __name__ == "__main__":
    f=open('langdata.py','w')
    langdata=dumpvoc(t)
    #loadvoc(t,langdata)
    langdata='langdata="\\\n'+langdata+'"\n'
    f.write(langdata)
    f.close()
