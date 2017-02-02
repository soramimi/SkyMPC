#!/usr/bin/ruby -Ks

Product = "SkyMPC"
CompanyName = "S.Fuchita"
Description = "a MPD client"
LegalCopyright = "Copyright (C) 2017 S.Fuchita (@soramimi_jp)"
OriginalFilename = "SkyMPC.exe"
Version = "1.6.2.0"

$version1 = "0"
$version2 = "0"
$version3 = "0"
$version4 = "0"
if Version =~ /([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)/
	$version1 = $1
	$version2 = $2
	$version3 = $3
	$version4 = $4
end


File.open("SkyMPC.rc", "w") {|file|
	file.puts <<___
#include <windows.h>

100 ICON DISCARDABLE "image/appicon.ico"

VS_VERSION_INFO     VERSIONINFO
  FILEVERSION       #{$version1},#{$version2},#{$version3},#{$version4}
  PRODUCTVERSION    #{$version1},#{$version2},#{$version3},#{$version4}
 FILEFLAGSMASK 0x3fL
 FILEFLAGS 0x0L
 FILEOS 0x4L
 FILETYPE 0x1L
 FILESUBTYPE 0x0L
BEGIN
	BLOCK "StringFileInfo"
	BEGIN
        BLOCK "041103a4"
		BEGIN
            VALUE "CompanyName", "#{CompanyName}"
            VALUE "FileDescription", "#{Description}"
            VALUE "FileVersion", "#{$version1}, #{$version2}, #{$version3}, #{$version4}"
			VALUE "InternalName",    "#{OriginalFilename}"
            VALUE "LegalCopyright", "#{LegalCopyright}"
			VALUE "OriginalFilename","#{OriginalFilename}"
            VALUE "ProductName", "#{Product}"
            VALUE "ProductVersion", "#{$version1}, #{$version2}, #{$version3}, #{$version4}"
		END
	END
	BLOCK "VarFileInfo"
	BEGIN
		VALUE "Translation", 0x0411, 932
    END
END
___
}
