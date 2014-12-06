#!/usr/bin/ruby
###########################################################################
#   Parses the Amarok debug log and prints methods which have unbalanced  #
#   BEGIN and END. This script only works for those methods with a        #
#   DEBUG_BLOCK.                                                          #
#                                                                         #
#   Copyright                                                             #
#   (C) 2010 Casey Link <unnamedrambler@gmail.com>                        #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
###########################################################################

# Use: cat amarok.log | beginend.rb

input = STDIN.read
stack = []

input.each_line do |line|
    begins = line.scan(/amarok:\s+BEGIN: (.*)/)
    if begins[0] != nil
      stack.push(begins[0][0].strip)
    end
    ends = line.scan(/amarok:\s+END__: (.*) - (.*)/)
    if ends[0] != nil 
      if !stack.include? ends[0][0].strip
        # could this ever happen?
	puts "Method ended but not begun: #{ends[0][0]}"
      else
        # only delete the most recent matching block
        last = stack.rindex(ends[0][0].strip)
        stack.delete_at(last)
      end
    end
end

# Any remaining methods blocks will be unbalanced
stack.each do |block|
  puts "Not ended: #{block}"
end

puts "Done."

