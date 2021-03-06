PROJECT_NAME = path.getname(os.getcwd())

minko.project.worker("minko-plugin-" .. PROJECT_NAME)

	removeplatforms { "html5" }

	files {
		"include/**.hpp",
		"src/**.cpp"
	}

	includedirs {
		"include",
		"lib/curl/include"
	}

	defines { "CURL_STATICLIB" }

	-- linux
	configuration { "linux32 or linux64" }
		links { "curl"}

	-- windows
	configuration { "windows32 or windows 64" }
		links { "libcurl" }
		
	-- macos
	configuration { "osx64" }
		links { "curl"}
