-------------------
-- CONFIGURATION --
-------------------
local buildExamples = true

--------------
-- GCC TOOL --
--------------
local gcc = premake.tools.gcc

gcc.tools =
{
	cc = "gcc",
	cxx = "g++",
	ar = "ar"
}

function gcc.gettoolname(cfg, tool)
	return gcc.tools[tool]
end

-----------------
-- ATEMA FILES --
-----------------
local projectDir = "../../"
local examplesDirs = os.matchdirs(projectDir .. "examples/*")
local modulesFiles = os.matchfiles("./Modules/*.lua")
local modulesLibs = {}

for k, moduleFile in pairs(modulesFiles) do
	table.insert(modulesLibs, "Atema" .. moduleFile:match(".*/(.*).lua"))
end

---------------
-- WORKSPACE --
---------------

function setDefaultFlags()
	flags({
		"MultiProcessorCompile",
		"ShadowedVariables",
		"UndefinedIdentifiers",
		"NoImplicitLink"
	})
end

print "Generating workspace"
local AtemaLibKind = "SharedLib" -- StaticLib or SharedLib
local WorkspaceDir = "../AtemaProject/"

workspace "Atema"
	configurations { "Debug", "Release" }
	cppdialect "C++17"
	setDefaultFlags()
	location (WorkspaceDir)
	
	if (AtemaLibKind == "StaticLib") then
		defines { "ATEMA_STATIC" }
	end

-------------
-- MODULES --
-------------
print "Generating modules"
for k, moduleFile in pairs(modulesFiles) do
	local moduleName = moduleFile:match(".*/(.*).lua")
	local projectName = "Atema" .. moduleName
	
	-- Generic module infos
	MODULE = {}
	
	MODULE.defines = {}
	MODULE.extlibs = {}
	MODULE.dependencies = {}
	MODULE.osDependencies = {}
	MODULE.osDependencies.windows = {}
	MODULE.osDependencies.posix = {}
	
	-- Loading the module infos for each module file
	local file, e = loadfile(moduleFile)
	
	if (file) then
		
		file()
		
		project(projectName)
			kind(AtemaLibKind)
			language "C++"
			setDefaultFlags()
			targetname(projectName)
			targetdir(projectDir .. "bin/%{cfg.buildcfg}")
			location(WorkspaceDir .. projectName)
			
			-- Source files
			local moduleFiles =
			{
				(projectDir .. "include/Atema/" .. moduleName .. "/**.hpp"),
				(projectDir .. "include/Atema/" .. moduleName .. "/**.inl"),
				(projectDir .. "include/Atema/" .. moduleName .. "/**.tpp"),
				
				(projectDir .. "src/Atema/" .. moduleName .. "/**.hpp"),
				(projectDir .. "src/Atema/" .. moduleName .. "/**.inl"),
				(projectDir .. "src/Atema/" .. moduleName .. "/**.tpp"),
				(projectDir .. "src/Atema/" .. moduleName .. "/**.cpp")
			}
			
			for k, extlib in pairs(MODULE.extlibs) do
				table.insert(moduleFiles, (projectDir .. "extlibs/include/" .. extlib .. "/**.hpp"))
				table.insert(moduleFiles, (projectDir .. "extlibs/include/" .. extlib .. "/**.h"))
				table.insert(moduleFiles, (projectDir .. "extlibs/include/" .. extlib .. "/**.inl"))
				table.insert(moduleFiles, (projectDir .. "extlibs/include/" .. extlib .. "/**.tpp"))
				table.insert(moduleFiles, (projectDir .. "extlibs/src/" .. extlib .. "/**.hpp"))
				table.insert(moduleFiles, (projectDir .. "extlibs/src/" .. extlib .. "/**.h"))
				table.insert(moduleFiles, (projectDir .. "extlibs/src/" .. extlib .. "/**.inl"))
				table.insert(moduleFiles, (projectDir .. "extlibs/src/" .. extlib .. "/**.tpp"))
				table.insert(moduleFiles, (projectDir .. "extlibs/src/" .. extlib .. "/**.cpp"))
				table.insert(moduleFiles, (projectDir .. "extlibs/src/" .. extlib .. "/**.c"))
			end
			
			files(moduleFiles)
			
			-- Include directories
			includedirs
			{
				(projectDir .. "include/"),
				(projectDir .. "extlibs/include/")
			}
			
			-- Dependencies
			local moduleLinks = {MODULE.dependencies}
			
			-- os : bsd, linux, macosx, solaris, or windows
			if (os.host() == "windows") then
				for k, lib in pairs(MODULE.osDependencies.windows) do
					table.insert(moduleLinks, lib)
				end
			else -- posix os (linux, macosx, bsd, solaris)
				for k, lib in pairs(MODULE.osDependencies.posix) do
					table.insert(moduleLinks, lib)
				end
			end
			
			links(moduleLinks)
			
			libdirs
			{
				(projectDir .. "extlibs/lib/"),
				(projectDir .. "extlibs/bin/")
			}
			
			-- Specific actions
			filter "configurations:Debug"
				defines { "ATEMA_DEBUG" }
				symbols "On"
				targetsuffix("-d")
			
			filter "configurations:Release"
				optimize "On"
			
			filter "kind:SharedLib"
				defines { "ATEMA_" .. string.upper(moduleName) .. "_EXPORT" }
			
			filter "kind:StaticLib"
				targetprefix("lib")
			
			filter {"kind:StaticLib", "system:Windows"}
				targetextension(".a")
		else
			print("Failed to load module file : " .. e)
		end
end

--------------
-- EXAMPLES --
--------------
if (buildExamples == true) then
	print "Generating examples"
	for k, exampleDir in pairs(examplesDirs) do
		local exampleName = exampleDir:match(".*/(.*)")
		
		project(exampleName)
			kind "ConsoleApp"
			language "C++"
			setDefaultFlags()
			targetdir(projectDir .. "bin/%{cfg.buildcfg}")
			targetname(exampleName)
			location(WorkspaceDir .. "Examples")
			
			includedirs
			{
				(projectDir .. "include/")
			}
			
			files
			{
				(exampleDir .. "/**.hpp"),
				(exampleDir .. "/**.cpp")
			}
			
			links(modulesLibs)
			
			-- Specific actions
			filter "configurations:Debug"
				defines { "ATEMA_DEBUG" }
				symbols "On"
				targetsuffix("-d")
			
			filter "configurations:Release"
				optimize "On"
	end	
end
