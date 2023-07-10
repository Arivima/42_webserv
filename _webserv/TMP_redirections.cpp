server {
  listen 80;
  server_name example.com;
  return 301 $scheme://www.example.com$request_uri;
}