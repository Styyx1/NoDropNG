-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsse-ng")

-- set project
set_project("drop-chances")
set_version("1.0.2")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("build.optimization.lto", true)
set_policy("package.requires_lock", true)

-- set configs
set_config("commonlib_json", true)
set_config("commonlib_toml", true)
set_config("rex_json", true)
set_config("rex_toml", true)

add_requires("nlohmann_json", {configs = {header_only = true}})

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- targets
target("drop-chances")
    -- add dependencies to target
    add_deps("commonlibsse-ng")
    add_packages("nlohmann_json")

    -- add commonlibsse-ng plugin
    add_rules("commonlibsse-ng.plugin", {
        name = "drop-chances",
        author = "styyx",
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
    add_includedirs("extern/clib-utils/include", {public = true})
