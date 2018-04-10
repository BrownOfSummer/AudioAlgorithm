from __future__ import print_function
import ctypes
# gcc libfuncs.c -o libfuncs.so -shared -fPIC
libfuncs = ctypes.cdll.LoadLibrary('libfuncs.so')
if __name__ == '__main__':
    res1 = libfuncs.func1(2,3)
    print("res1 = %d" % res1)
    res2 = libfuncs.func2(2,3)
    print("res2 = %s" % res2)
    libfuncs.func3(100, 1)
    #res4 = libfuncs.func4(1,2.1)
    #print(res4)
