from common import *

shell(f'{CMAKE} --build {BUILDDIR}/ngfx --config {BUILD_TYPE} -j8')
