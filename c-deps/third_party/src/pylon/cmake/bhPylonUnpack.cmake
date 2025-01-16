# Unpack pylon package contents.
#
# The downloaded archive is automatically extracted, however, the actual
# contents of the package are contained within another archive.

file(ARCHIVE_EXTRACT
	INPUT "${bh_pylon_srcdir}/${bh_pylon_archive}"
	DESTINATION "${bh_pylon_bindir}"
)
