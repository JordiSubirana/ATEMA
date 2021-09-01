-- Additionnal defines of the module
MODULE.defines = {}

-- Thirdparty embedded sources
MODULE.thirdparty = {}

-- Dependencies (Atema modules & compiled thirdparty libraries)
MODULE.dependencies = {"AtemaMath", "glfw3"}

-- OS specific dependencies (compiled)
MODULE.osDependencies.windows = {"gdi32"}
MODULE.osDependencies.posix = {}