-------------------
-- CONFIGURATION --
-------------------
local addExamples = true

-----------------
-- ATEMA FILES --
-----------------
local atemaDir = "./"
local includeDir = atemaDir .. "include/"
local srcDir = atemaDir .. "src/"
local thirdPartyDir = atemaDir .. "thirdparty/"
local thirdPartyIncludeDir = thirdPartyDir .. "include/"
local targetDir = atemaDir .. "bin/$(plat)_$(arch)_$(mode)"
local exampleDir = atemaDir .. "examples/"

------------------
-- MODULES DATA --
------------------
local modules =
{
	Core =
	{
		windowsDependencies = {"winmm"}
	},
	Graphics =
	{
		dependencies = {"AtemaCore", "AtemaMath", "AtemaRenderer", "AtemaShader"}
	},
	Math =
	{
		
	},
	Renderer =
	{
		dependencies = {"AtemaCore", "AtemaMath", "AtemaShader", "AtemaWindow"}
	},
	Shader =
	{
		dependencies = {"AtemaCore", "AtemaMath"},
		packages = {"glslang"}
	},
	UI =
	{
		dependencies = {"AtemaCore", "AtemaMath", "AtemaRenderer", "AtemaVulkanRenderer", "AtemaWindow"},
		packages = {"glfw"},
		windowsDependencies = {"gdi32"}
	},
	VulkanRenderer =
	{
		dependencies = {"AtemaCore", "AtemaMath", "AtemaRenderer", "AtemaShader", "AtemaWindow"}
	},
	Window =
	{
		dependencies = {"AtemaCore", "AtemaMath"},
		packages = {"glfw"},
		windowsDependencies = {"gdi32"}
	}
}

-----------------
-- MAIN SCRIPT --
-----------------
set_xmakever("2.6.5")

set_project("Atema")

add_requires("glfw", {configs = {shared = true}})
add_requires("glslang")

add_includedirs(includeDir)
add_sysincludedirs(thirdPartyIncludeDir)
set_rundir(targetDir)
set_targetdir(targetDir)

set_languages("c99", "cxx17")

set_allowedplats("windows")
set_allowedmodes("debug", "releasedbg", "release")

set_defaultmode("debug")

add_rules("mode.debug", "mode.releasedbg", "mode.release")

if (is_mode("debug")) then
	add_defines("ATEMA_DEBUG")
end

-- Modules
for moduleName, module in pairs(modules) do
	
	target("Atema" .. moduleName)
	
	set_kind("shared")
	
	add_headerfiles(includeDir .. "Atema/" .. moduleName .. "/**.hpp")
	add_headerfiles(includeDir .. "Atema/" .. moduleName .. "/**.inl")
	
	add_files(srcDir .. "Atema/" .. moduleName .. "/**.cpp")
	
	if (module.dependencies) then
		
		add_deps(module.dependencies)
		
	end
	
	if (module.packages) then
		
		add_packages(module.packages)
		
	end
	
	if (is_plat("windows") and module.windowsDependencies) then
		
		add_syslinks(module.windowsDependencies)
		
	end
	
	add_defines("ATEMA_" .. moduleName:upper() .. "_EXPORT")
	
end

-- Examples
if (addExamples == true) then
	
	local exampleDirs = os.dirs(exampleDir .. "*")
	
	for k, examplePath in pairs(exampleDirs) do
		
		local exampleName = examplePath:match(".*[\\/](.*)")
		
		target(exampleName)
		
		set_group("Examples")
	
		set_kind("binary")
		
		add_headerfiles(examplePath .. "/**.hpp")
		add_headerfiles(examplePath .. "/**.inl")
		
		add_files(examplePath .. "/**.cpp")
		
		add_filegroups(exampleName, {rootdir = examplePath})
		
		for moduleName, module in pairs(modules) do
			
			add_deps("Atema" .. moduleName)
			
		end
		
	end
	
end