#!/usr/bin/perl

use strict;
use warnings;

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

sub generate_autoindex {
	my $dir = $ENV{'ROOT'} . $ENV{'QUERY_STRING'};
	my $rel_dir = $ENV{'QUERY_STRING'};

	eval {
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
		$html .= "<title>Index of $dir</title>\n";
		$html .= "</head>\n";
		$html .= "<body>\n";
		$html .= "<h1>Index of $dir</h1>\n";
		$html .= "<ul>\n";

		foreach my $entry (@entries) {
			next if $entry =~ /^\./;  # Skip hidden files/directories

			my $path = "$dir/$entry";
			my $size = -d $path ? '-' : -s _;  # Use '-' for directories

			$html .= "<li><a href=\"/cgi-bin/autoindex.pl?/$rel_dir/$entry\">$entry</a> ($size bytes)</li>\n";
		}

		$html .= "</ul>\n";
		$html .= "</body>\n";
		$html .= "</html>\n";

		closedir($dh);

		# Calculate the content length
		my $content_length = length($html);

		# Print the HTTP response headers
		my $headers = $ENV{'SERVER_PROTOCOL'} . " 200 OK\r\n";
		$headers .= "Content-Type: text/html\r\n";
		$headers .= "Content-Length: $content_length\r\n";
		$headers .= "\r\n";
		
		return ($headers . $html);
	}

	if ($@) {
		open(my $file, '<', $dir) or die "Failed to open file or directory";

		my $content_type = determine_content_type($dir);

		# Print the HTTP response headers
		my $headers = $ENV{'SERVER_PROTOCOL'} . " 200 OK\r\n";
		$headers .= "Content-Type: $content_type\r\n";
		$headers .= "Content-Length: $content_length\r\n";
		$headers .= "\r\n";
		
		binmode($file);
		binmode(STDOUT);
		print while <$file>;
		close($file);


	}

}

# Get the requested directory from the environment variable
my $requested_dir = $ENV{'ROOT'} . $ENV{'REQUEST_URI'};

# Generate the directory listing
my $autoindex_html = generate_autoindex($requested_dir);

# Print the directory listing
print $autoindex_html;