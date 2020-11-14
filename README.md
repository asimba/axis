# axis  
  
Axis (eXtra Simple - XS). This program provides simple cryptographic protection of user data.  
  
--- 
Usage : ./bin/axis \<OPTIONS\>  
**Examples:**  
```  
axis --encrypt -i ./ --password=abcdef > encrypted.xs  
axis --encrypt -p abcdef --input=./source.in --output=./encrypted.xs  
axis --encrypt -p abcdef -i ./src1 -i ./src2 -o ./encrypted.xs  
axis --decrypt -p abcdef -i ./source.xs  
axis --decrypt -p abcdef -i ./source.xs -o ./  
axis -e -p abcdef -i ./source.in -o ./encrypted.xs  
axis -e -j -s -p abcdef -i "./source.*" -o ./encrypted.xs  
axis --statistics -e -p abcdef -i ./source.in -o ./encrypted.xs  
axis -d -i ./source.xs  
axis -s -d -i ./source.xs  
axis --repair -i ./source.xs --output=./repaired.xs  
axis -r -i ./source.xs -o ./repaired.xs  
cat source.xs | axis --repair > repaired.xs  
axis --check -i ./source.xs  
axis -c -i ./source.xs  
cat source.xs | axis --check  
axis --list -i ./source.xs -p abcdef  
axis -l -i ./source.xs  
axis -u -i "./*.in"  
axis -u"./*.in"  
cat source.in | axis -u  
```  
---  
**Options**:  
Option  | Value
----- | ------ 
-e, --encrypt | encode input data ("encrypt" MODE)  
-d, --decrypt | decode input data ("decrypt" MODE)  
-d0, --decrypt=0 | decode input data ("decrypt" MODE), but without writing - a kind of check  
-l, --list | list contents of the XS-archive  
-l0, --list=0 | list contents of the XS-archive, but without printing - a kind of check  
-c, --check | check integrity of the encoded data  
-r, --repair | repair corrupted XS-data  
-u\[NAME\], --checksum\[=NAME\] | calculate Axis (160-bit) checksums - ONLY ONE FILENAME or PATH  
-p STRING, --password=STRING | max size of the STRING: 255 characters<br> min size of the STRING:  6  bytes<br> (required if MODE is NOT "check" or "repair";                       if it is not set - it will be requested)  
-n, --ignore | ignore impossibility of change file ownership/permissions or access/modification times (ONLY for "decrypt" MODE)  
-g, --incognito | don't write/check identification signature in XS-archive  
-m, --irreplaceable | don't write/check recovery data in XS-archive (ONLY for "encrypt" or "decrypt" MODE)<br>WARNING: HIGH probability of damage to the                            output data!  
-j, --jam | synonymous for "-g -m"  
-i NAME  , --input=NAME | input file or directory (wildcards accepted ONLY for "encrypt" MODE)<br>NOTE: when using wildcards - put them in                              double quotes  
-o NAME  , --output=NAME | output file or directory - ONLY ONE FILENAME or PATH  
-s, --statistics | display work progress  
-h, --help | display this help and exit  
-v, --version | output version information and exit  
  
---  
  
<p align="justify">This is free software. There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
WARNING: This program can be used ONLY for protection of personal information; it CANNOT be used for protection of confidential information (personally identifiable information etc.) or any other data.</p>    
  
---  
  
Written by Alexey V. Simbarsky.  
