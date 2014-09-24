Pod::Spec.new do |s|
  s.name         = "lua52"
  s.version      = "0.0.2"
  s.summary      = "lua52 library."
  s.description  = <<-DESC
                   lua5.2.3.
                   DESC
  s.homepage     = "https://github.com/HydraFramework/lua51"
  s.license      = "MIT"
  s.author       = { "samchang" => "sam.chang@me.com" }
  s.platform     = :ios, "5.1.1"
  s.source       = { :git => "https://github.com/HydraFramework/lua51.git", :tag => "v0.0.2" }
  s.source_files  = "lua-5.2.3/src/*.{h,c}", "lua52/*.{h,c}"
  s.exclude_files = "lua-5.2.3/src/lua.c", "lua-5.2.3/src/luac.c"
  s.compiler_flags  = '-DLUA_USER_H="\"luauser.h\"" -DLUA_COMPAT_ALL -DLUA_COMPAT_MODULE'
  s.header_dir = 'lua52'
end