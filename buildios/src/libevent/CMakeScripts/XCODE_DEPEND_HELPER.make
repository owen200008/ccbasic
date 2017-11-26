# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.event.Debug:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent.a


PostBuild.event_core.Debug:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_core.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_core.a


PostBuild.event_extra.Debug:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_extra.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Debug/libevent_extra.a


PostBuild.event.Release:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent.a


PostBuild.event_core.Release:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_core.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_core.a


PostBuild.event_extra.Release:
/Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_extra.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/libevent/lib/Release/libevent_extra.a




# For each target create a dummy ruleso the target does not have to exist
