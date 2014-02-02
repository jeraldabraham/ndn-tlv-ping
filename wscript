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
        target   = 'ndn-tlv-ping',
        source   = 'ndn-tlv-ping.cpp',
        use      = 'NDN_CPP',
        )

    bld.program (
        features = 'cxx',
        target   = 'ndn-tlv-pingserver',
        source   = 'ndn-tlv-pingserver.cpp',
        use      = 'NDN_CPP',
        )
