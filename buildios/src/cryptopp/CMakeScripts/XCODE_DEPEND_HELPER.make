# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.cryptopp-static.Debug:
/Volumes/data/github/ccbasic/buildios/src/cryptopp/Debug/libcryptopp.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/cryptopp/Debug/libcryptopp.a


PostBuild.cryptopp-static.Release:
/Volumes/data/github/ccbasic/buildios/src/cryptopp/Release/libcryptopp.a:
	/bin/rm -f /Volumes/data/github/ccbasic/buildios/src/cryptopp/Release/libcryptopp.a




# For each target create a dummy ruleso the target does not have to exist
