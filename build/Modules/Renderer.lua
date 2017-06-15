-- Additionnal defines of the module
MODULE.defines = {}

-- Extlibs (third party embedded sources)
MODULE.extlibs = {}

-- Dependencies (Atema modules & compiled extlibs)
MODULE.dependencies = {"AtemaCore"}

-- OS specific dependencies (compiled)
MODULE.osDependencies.windows = {"gdi32", "opengl32"} -- for windowing system
MODULE.osDependencies.posix = {"GL", "X11", "dl"} -- for windowing system