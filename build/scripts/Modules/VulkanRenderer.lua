-- Additionnal defines of the module
MODULE.defines = {}

-- Thirdparty embedded sources
MODULE.thirdparty = {}

-- Dependencies (Atema modules & compiled thirdparty libraries)
MODULE.dependencies = {"AtemaCore", "AtemaRenderer"}

-- OS specific dependencies (compiled)
MODULE.osDependencies.windows = {"vulkan-1"}
MODULE.osDependencies.posix = {}