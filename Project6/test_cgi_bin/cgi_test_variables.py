#!/usr/bin/python3

import os
import sys
import cgi
import cgitb
cgitb.enable()

form = cgi.FieldStorage()
s = ''
for i in range( int(os.environ['CONTENT_LENGTH']) ):
    s += sys.stdin.read(1)

print ( s )
print ("a:", form["a"].value)
print ("b:", form["b"].value)

#print ("Entire field storage")
#print (form)
