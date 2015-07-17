echo "Setting up xdaqgem variables"
setenv PATH ${PATH}:/sbin
setenv PATH ${PATH}:/usr/sbin
setenv PATH ${PATH}:/usr/local/sbin

setenv uHALROOT /opt/cactus
setenv PATH ${PATH}:${uHALROOT}/bin
setenv PATH ${PATH}:/data/sources/glib_v3/sw/PyChips/scripts
setenv LD_LIBRARY_PATH ${uHALROOT}/lib
setenv PYTHONPATH /data/sources/glib_v3/sw/PyChips/src

setenv XDAQ_ROOT /opt/xdaq
setenv XDAQ_DOCUMENT_ROOT ${XDAQ_ROOT}/htdocs
setenv PATH ${PATH}:${XDAQ_ROOT}/bin
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib
