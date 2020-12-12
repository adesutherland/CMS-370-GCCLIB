cmake_minimum_required(VERSION 3.17)
project(CMS_370_GCCLIB C)

set(CMAKE_C_STANDARD 90)

include_directories(.)

add_executable(CMS_370_GCCLIB
        assert.c
        assert.h
        cmsio.c
        cmsrtstb.c
        cmsruntm.c
        cmsruntm.h
        cmsstdio.c
        cmsstdlb.c
        cmssys.h
        cmssysc.c
        condrv.c
        ctype.c
        ctype.h
        dskdrv.c
        errno.h
        float.h
        gcccrab.h
        limits.h
        locale.c
        locale.h
        makelib.c
        malloc.c
        math.c
        math.h
        mtest.c
        prtdrv.c
        pundrv.c
        rdrdrv.c
        resmain.c
        rexxsaa.c
        rexxsaa.h
        setjmp.h
        signal.c
        signal.h
        stdarg.h
        stddef.h
        stdio.h
        stdlib.h
        string.c
        string.h
        test.c
        teststak.c
        time.c
        time.h
        tstarg1.c
        tstarg2.c
        tstcmssy.c
        tstconio.c
        tstfdkio.c
        tstio.c
        tstprtio.c
        tstpunio.c
        tstrdrio.c
        tsts.c
        tsts.h
        tstvdkio.c)
