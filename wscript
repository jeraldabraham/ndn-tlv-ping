# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION='0.3'
NAME="ndn-tlv-ping"

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load("compiler_cxx")
    conf.check_cfg(package='libndn-cpp-dev', args=['--cflags', '--libs'], uselib_store='NDN_CPP', mandatory=True)

def build (bld):
    bld.program (
        features = 'cxx',
        target   = 'ndnping',
        source   = 'ndnping.cpp',
        use      = 'NDN_CPP',
        )

    bld.program (
        features = 'cxx',
        target   = 'ndnpingserver',
        source   = 'ndnpingserver.cpp',
        use      = 'NDN_CPP',
        )
