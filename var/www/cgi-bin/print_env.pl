
#!/usr/bin/perl

use strict;
use warnings;


my $html = "<!DOCTYPE html>\n";
$html .= "<html>\n";
$html .= "<head>\n";
$html .= "<script type=\"text/javascript\">\
function goBack() {\
    history.back();\
}\
</script>";

$html .= "<style>";
$html .= "table {";
$html .= "border-collapse: collapse;";
$html .= "width: 100%;";
$html .= "}";
$html .= "th, td {";
$html .= "border: 1px solid black;";
$html .= "padding: 8px;";
$html .= "text-align: left;";
$html .= "}";
$html .= "th {";
$html .= "background-color: \#dddddd;";
$html .= "}";
$html .= "</style>";

$html .= "<title>CGI ENV</title>\n";
$html .= "</head>\n";
$html .= "<body>\n";
$html .= "<h1>CGI ENV</h1>\n";


$html .= "<table>";
$html .= "<tr><th>Line</th><th>Variable</th><th>Value</th></tr>";  # Header row
my $line_number = 1;
foreach my $key (sort keys %ENV) {
	$html .= "<tr><td>$line_number</td><td>$key</td><td>$ENV{$key}</td></tr>";
	$line_number ++;
}
$html .= "</table>";


$html .= "<button onclick='goBack()'>\<==</button>";
$html .= "</body>\n";
$html .= "</html>\n";

my $content_length = length($html);

print	$ENV{'SERVER_PROTOCOL'};
print	" 200 OK\r\n";
print	"Content-Type: text/html\r\n";
print	"Content-Length: $content_length\r\n";
print	"\r\n";

print	$html;
