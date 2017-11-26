# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.basicTest.Debug:
PostBuild.event.Debug: /Volumes/data/github/ccbasic/buildios/Debug/basicTest
PostBuild.basiclib.Debug: /Volumes/data/github/ccbasic/buildios/Debug/basicTest
PostBuild.cryptopp-static.Debug: /Volumes/data/github/ccbasic/buildios/Debug/basicTest
PostBuild.event.Debug: /Volumes/data/github/ccbasic/buildios/Debug/basicTest
PostBuild.zlib.Debug: /Volumes/data/github/ccbasic/buildios/Debug/basicTest
/Volumes/data/github/ccbasic/buildios/Debug/basicTest:\
	/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a\
	/Volumes/data/github/ccbasic/buildios/src/Debug/libbasiclib.a\
	/Volumes/data/github/ccbasic/buildios/src/cryptopp/Debug/libcryptopp.a\
	/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a\
	/Volumes/data/github/ccbasic/buildios/src/zlib/Debug/libz.1.2.11.dylib
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/Debug/basicTest


PostBuild.basiclib.Debug:
/Volumes/data/github/ccbasic/buildios/src/Debug/libbasiclib.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/Debug/libbasiclib.a


PostBuild.event.Debug:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a


PostBuild.event_core.Debug:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_core.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_core.a


PostBuild.event_extra.Debug:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_extra.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_extra.a


PostBuild.cryptopp-static.Debug:
/Volumes/data/github/ccbasic/buildios/src/cryptopp/Debug/libcryptopp.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/cryptopp/Debug/libcryptopp.a


PostBuild.zlib.Debug:
/Volumes/data/github/ccbasic/buildios/src/zlib/Debug/libz.dylib:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/zlib/Debug/libz.dylib


PostBuild.lualib.Debug:
/Volumes/data/github/ccbasic/buildios/src/lualib/Debug/liblualib.dylib:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/lualib/Debug/liblualib.dylib


PostBuild.basicTest.Release:
PostBuild.event.Release: /Volumes/data/github/ccbasic/buildios/Release/basicTest
PostBuild.basiclib.Release: /Volumes/data/github/ccbasic/buildios/Release/basicTest
PostBuild.cryptopp-static.Release: /Volumes/data/github/ccbasic/buildios/Release/basicTest
PostBuild.event.Release: /Volumes/data/github/ccbasic/buildios/Release/basicTest
PostBuild.zlib.Release: /Volumes/data/github/ccbasic/buildios/Release/basicTest
/Volumes/data/github/ccbasic/buildios/Release/basicTest:\
	/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a\
	/Volumes/data/github/ccbasic/buildios/src/Release/libbasiclib.a\
	/Volumes/data/github/ccbasic/buildios/src/cryptopp/Release/libcryptopp.a\
	/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a\
	/Volumes/data/github/ccbasic/buildios/src/zlib/Release/libz.1.2.11.dylib
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/Release/basicTest


PostBuild.basiclib.Release:
/Volumes/data/github/ccbasic/buildios/src/Release/libbasiclib.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/Release/libbasiclib.a


PostBuild.event.Release:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a


PostBuild.event_core.Release:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_core.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_core.a


PostBuild.event_extra.Release:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_extra.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_extra.a


PostBuild.cryptopp-static.Release:
/Volumes/data/github/ccbasic/buildios/src/cryptopp/Release/libcryptopp.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/cryptopp/Release/libcryptopp.a


PostBuild.zlib.Release:
/Volumes/data/github/ccbasic/buildios/src/zlib/Release/libz.dylib:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/zlib/Release/libz.dylib


PostBuild.lualib.Release:
/Volumes/data/github/ccbasic/buildios/src/lualib/Release/liblualib.dylib:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/lualib/Release/liblualib.dylib




# For each target create a dummy ruleso the target does not have to exist
/Volumes/data/github/ccbasic/buildios/src/Debug/libbasiclib.a:
/Volumes/data/github/ccbasic/buildios/src/Release/libbasiclib.a:
/Volumes/data/github/ccbasic/buildios/src/cryptopp/Debug/libcryptopp.a:
/Volumes/data/github/ccbasic/buildios/src/cryptopp/Release/libcryptopp.a:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a:
/Volumes/data/github/ccbasic/buildios/src/zlib/Debug/libz.1.2.11.dylib:
/Volumes/data/github/ccbasic/buildios/src/zlib/Release/libz.1.2.11.dylib:
