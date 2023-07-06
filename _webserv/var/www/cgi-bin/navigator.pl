# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    navigator.pl                                       :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/06 03:50:02 by mmarinel          #+#    #+#              #
#    Updated: 2023/07/06 19:07:28 by mmarinel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/usr/bin/perl

use strict;
use warnings;

sub navigator {
	my $dir = $ENV{'ROOT'} . $ENV{'QUERY_STRING'};
	my $rel_dir = $ENV{'QUERY_STRING'};

	eval {
		# input sanitization
		if (-1 != index($rel_dir, "..")) {
			print_error_page(404, "Not Found");
		}
		
		# Open the directory
		opendir(my $dh, $dir) or die;

		# Read the directory entries
		my @entries = readdir($dh);

		# Sort the entries (directories first, then files)
		@entries = sort { -d "$dir/$a" <=> -d "$dir/$b" || lc($a) cmp lc($b) } @entries;

		# Generate the HTML for the directory listing
		my $html = "<!DOCTYPE html>\n";
		$html .= "<html>\n";
		$html .= "<head>\n";
		$html .= "<script type=\"text/javascript\">\
        function goBack() {\
            history.back();\
        }\
    </script>";
		$html .= "<title>Index of $dir</title>\n";
		$html .= "</head>\n";
		$html .= "<body>\n";
		$html .= "<h1>Index of $dir</h1>\n";
		$html .= "<ul>\n";

		foreach my $entry (@entries) {
			next if $entry =~ /^\./;  # Skip hidden files/directories

			my $path = "$dir/$entry";
			my $size = -d $path ? '-' : -s _;  # Use '-' for directories

			$html .= "<li><a href=\"/cgi-bin/navigator.pl?/$rel_dir/$entry\">$entry</a> ($size bytes)</li>\n";
		}

		$html .= "</ul>\n";
		$html .= "<button onclick='goBack()'>\<==</button>";
		$html .= "</body>\n";
		$html .= "</html>\n";

		closedir($dh);

		# Calculate the content length
		my $content_length = length($html);

		# Print the HTTP response headers
		print	$ENV{'SERVER_PROTOCOL'};
		print	" 200 OK\r\n";
		print	"Content-Type: text/html\r\n";
		print	"Content-Length: $content_length\r\n";
		print	"\r\n";
		
		print	$html;

		exit;
	};
	if ($@) {
		open(my $file, '<', $dir) or print_error_page(404, "Not Found");

		my $content_type = determine_content_type($dir);
		my $content_length = -s $dir;
		my $last_slash_pos = rindex($rel_dir, "/");
		my $file_name;
		if (-1 != $last_slash_pos) {
			$file_name = substr($rel_dir, $last_slash_pos + 1);
		}
		else {
			$file_name = $rel_dir;
		}

		# Print the HTTP response headers
		print	$ENV{'SERVER_PROTOCOL'};
		print	" 200 OK\r\n";
		print	"Content-Type: $content_type\r\n";
		print	"Content-Length: $content_length\r\n";
		print	"Content-Disposition: attachment; filename=\"$file_name\"\r\n";
		print	"\r\n";

		binmode($file);
		binmode(STDOUT);
		print while <$file>;
		close($file);

		exit;
	}

}

# Get the requested directory from the environment variable
my $requested_dir = $ENV{'ROOT'} . $ENV{'REQUEST_URI'};

navigator($requested_dir);




## Helper Functions

sub	print_error_page {
	my	($http_error, $msg) = @_;
	my	$body = "<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\
    <title> $http_error $msg </title>\
  </head>\
  <body>\
    <center><h1>PiouPiou Webserv</h1><h2> $http_error $msg </h2></center>\
  </body>\
</html>\
";
	my	$body_size = length($body);

	print	$ENV{'SERVER_PROTOCOL'};
	print	" $http_error $msg\r\n";
	print	"Content-Type: text/html\r\n";
	print	"Content-Length: $body_size\r\n";
	print	"\r\n";

	print $body;

	exit;
}

sub determine_content_type {
	my ($filepath) = @_;

	# Mapping of file extensions to content types
	my %content_types = (
	    'txt'  => 'text/plain',
	    'html' => 'text/html',
	    'css'  => 'text/css',
	    'js'  => 'application/js',
	    'pdf'  => 'application/pdf',
	    'png'  => 'img/png',
	    'jpg'  => 'img/jpg',
	    'jpeg'  => 'img/jpeg',
	    # Add more file extensions and corresponding content types as needed
	);

	my ($file_extension) = $filepath =~ /\.([^.]+)$/;

	return $content_types{$file_extension} || 'text/plain';
}
