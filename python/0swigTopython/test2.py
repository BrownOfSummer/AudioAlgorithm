from __future__ import print_function
import libfuncs
# (1) get libfuncs_wrap.c and libfuncs.py for import.
#       swig -python libfuncs.i
# (2) get .so for import by libfuncs.py
#       gcc -Wall -g libfuncs.c libfuncs_wrap.c -I /usr/include/python2.7 -shared -fPIC -lpython2.7 -o _libfuncs.s
if __name__ == '__main__':
    res1 = libfuncs.func1(2,3)
    print("res1 = %d" % res1)
    res2 = libfuncs.func2(2,3)
    print("res2 = %f" % res2)
    res3 = libfuncs.func3(100, 1) # void for None
    res4 = libfuncs.func4(1,2.1)
    print("[%d, %.3f]" % (res4.a, res4.b))
