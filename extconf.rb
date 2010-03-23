# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

# Give it a name
extension_name = 'rubyk8055'

# The destination
dir_config('rubyk8055')

have_library("usb")

# Do the work
create_makefile('rubyk8055')

