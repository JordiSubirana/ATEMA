-- Additionnal defines of the module
MODULE.defines = {}

-- Extlibs (third party embedded sources)
MODULE.extlibs = {"glad"}

-- Dependencies (Atema modules & compiled extlibs)
MODULE.dependencies = {"AtemaCore", "AtemaRenderer"}

-- OS specific dependencies (compiled)
MODULE.osDependencies.windows = {"gdi32", "opengl32"}
MODULE.osDependencies.posix = {"GL", "X11", "dl"}