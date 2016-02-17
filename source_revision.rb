#!/usr/bin/ruby -Ks


def get_revision()
	rev = ""
	if Dir.exist?(".svn")
		info = `svn info -r HEAD`
		info.each_line {|line|
			if line =~ /^Revision: ([0-9]+)/
				rev = $1
			end
		}
	elsif Dir.exist?(".git")
		hash = `git rev-parse HEAD`
		if hash =~ /^[0-9A-Za-z]+/
			rev = hash[0, 7]
		end
	end
	return rev
end

File.open("source_revision.c", "w") {|file|
	file.puts <<____
char const source_revision[] = "#{get_revision()}";
____
}

